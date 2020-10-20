#include "converters/angelbuilder.h"

#include <log.h>
#include <bson.h>

#include <angelscript.h>

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QDebug>

#include "angelsystem.h"
#include "components/angelbehaviour.h"

#define DATA    "Data"
#define GET     "get_"

static QHash<uint32_t, QImage> itemIcons = {
    {AngelClassMapModel::AngelItem::Module,     QImage()},
    {AngelClassMapModel::AngelItem::Class,      QImage(":/Images/icons/class.png")},
    {AngelClassMapModel::AngelItem::Method,     QImage(":/Images/icons/method.png")},
    {AngelClassMapModel::AngelItem::Property,   QImage(":/Images/icons/property.png")},
    {AngelClassMapModel::AngelItem::Enum,       QImage(":/Images/icons/enum.png")}
};

static QHash<asETypeIdFlags, QString> baseTypes = { {asTYPEID_VOID,   "void"},
                                                    {asTYPEID_BOOL,   "bool"},
                                                    {asTYPEID_INT8,   "int8_t"},
                                                    {asTYPEID_INT16,  "int16_t"},
                                                    {asTYPEID_INT32,  "int32_t"},
                                                    {asTYPEID_INT64,  "int64_t"},
                                                    {asTYPEID_UINT8,  "uint8_t"},
                                                    {asTYPEID_UINT16, "uint16_t"},
                                                    {asTYPEID_UINT32, "uint32_t"},
                                                    {asTYPEID_UINT64, "uint64_t"},
                                                    {asTYPEID_FLOAT,  "float"},
                                                    {asTYPEID_DOUBLE, "double"}
                                                  };

class CBytecodeStream : public asIBinaryStream {
public:
    CBytecodeStream(ByteArray &ptr) :
        array(ptr) {

    }
    int Write(const void *ptr, asUINT size) {
        if(size == 0) {
            return 0;
        }
        uint32_t offset = array.size();
        array.resize(offset + size);
        memcpy(&array[offset], ptr, size);

        return static_cast<int>(size);
    }
    int Read(void *ptr, asUINT size) {
        A_UNUSED(ptr);
        A_UNUSED(size);
        return 0;
    }
protected:
    ByteArray &array;
};

VariantMap AngelSerial::saveUserData() const {
    VariantMap result;

    result[DATA]  = m_Array;

    return result;
}

AngelBuilder::AngelBuilder(AngelSystem *system) :
        m_pSystem(system),
        m_pScriptEngine(asCreateScriptEngine()),
        m_pClassModel(new AngelClassMapModel(m_pScriptEngine)) {

    m_pScriptEngine->SetMessageCallback(asFUNCTION(messageCallback), nullptr, asCALL_CDECL);
}

AngelBuilder::~AngelBuilder() {
    m_pScriptEngine->ShutDownAndRelease();
}

void AngelBuilder::init() {
    m_pSystem->registerClasses(m_pScriptEngine);
}

bool AngelBuilder::buildProject() {
    if(m_Outdated) {
        asIScriptModule *mod = m_pScriptEngine->GetModule("module", asGM_CREATE_IF_NOT_EXISTS);

        QFile base(":/Behaviour.txt");
        if(base.open( QIODevice::ReadOnly)) {
            mod->AddScriptSection("AngelData", base.readAll());
            base.close();
        }
        for(QString it : m_Sources) {
            QFile file(it);
            if(file.open( QIODevice::ReadOnly)) {
                mod->AddScriptSection("AngelData", file.readAll().data());
                file.close();
            }
        }

        if(mod->Build() >= 0) {
            QFile dst(m_Destination);
            if(dst.open( QIODevice::WriteOnly)) {
                AngelSerial serial;
                serial.m_Array.clear();
                CBytecodeStream stream(serial.m_Array);
                mod->SaveByteCode(&stream);

                ByteArray data = Bson::save( Engine::toVariant(&serial) );
                dst.write(reinterpret_cast<const char *>(&data[0]), data.size());
                dst.close();
            }
            // Do the hot reload
            m_pSystem->reload();

            m_pClassModel->update();
        }

        m_Outdated = false;
    }
    return true;
}

QAbstractItemModel *AngelBuilder::classMap() const {
    return m_pClassModel;
}

uint8_t AngelBuilder::convertFile(IConverterSettings *settings) {
    QFileInfo info(settings->absoluteDestination());

    m_Destination = info.absolutePath() + "/" + persistentUUID();

    return IBuilder::convertFile(settings);
}

QString AngelBuilder::builderVersion() {
    return "1.0";
}

const QString AngelBuilder::persistentAsset() const {
    return "AngelBinary";
}

const QString AngelBuilder::persistentUUID() const {
    return "{00000000-0101-0000-0000-000000000000}";
}

void AngelBuilder::messageCallback(const asSMessageInfo *msg, void *param) {
    A_UNUSED(param);
    Log(static_cast<Log::LogTypes>(msg->type)) << msg->section << "(line:" << msg->row << "col:" << msg->col << "):" << msg->message;
}


AngelClassMapModel::AngelClassMapModel(asIScriptEngine *engine) :
        m_pEngine(engine),
        m_pRootItem(new AngelClassItem({})) {

}

void AngelClassMapModel::update() {
    delete m_pRootItem;
    m_pRootItem = new AngelClassItem({});

    QHash<QString, AngelClassItem *> nameSpace;
    for(uint32_t g = 0; g < m_pEngine->GetGlobalFunctionCount(); g++) {
        asIScriptFunction *function = m_pEngine->GetGlobalFunctionByIndex(g);
        if(function) {
            QString name(function->GetNamespace());
            if(!name.isEmpty()) {
                AngelClassItem *classItem = nameSpace.value(name, nullptr);
                if(classItem == nullptr) {
                    classItem = new AngelClassItem({name, itemIcons.value(AngelClassMapModel::AngelItem::Class)}, m_pRootItem);
                    m_pRootItem->appendChild(classItem);
                    nameSpace[function->GetNamespace()] = classItem;
                }

                classItem->appendChild(new AngelClassItem({function->GetName(),
                                                           itemIcons.value(AngelClassMapModel::AngelItem::Method),
                                                           exportParams(function)}, classItem));
            }
        }
    }

    for(uint32_t i = 0; i < m_pEngine->GetObjectTypeCount(); i++) {
        asITypeInfo *info = m_pEngine->GetObjectTypeByIndex(i);
        if(info) {
            exportType(info);
        }
    }

    for(uint32_t e = 0; e < m_pEngine->GetEnumCount(); e++) {
        asITypeInfo *info = m_pEngine->GetEnumByIndex(e);
        if(info) {
            exportType(info, AngelItem::Enum);
        }
    }

    for(uint32_t m = 0; m < m_pEngine->GetModuleCount(); m++) {
        asIScriptModule *module = m_pEngine->GetModuleByIndex(m);
        if(module) {
            for(uint32_t i = 0; i < module->GetObjectTypeCount(); i++) {
                asITypeInfo *info = module->GetObjectTypeByIndex(i);
                if(info) {
                    exportType(info);
                }
            }
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void AngelClassMapModel::exportType(asITypeInfo *info, AngelItem type) {
    static QStringList dropList = {"opCast", "opImplCast", "set_"};
    static QHash<QString, QString> replaceMap = { {"opAssign",    "operator ="},
                                                  {"opAddAssign", "operator +="},
                                                  {"opSubAssign", "operator -="},
                                                  {"opMulAssign", "operator *="},
                                                  {"opDivAssign", "operator /="},
                                                  {"opModAssign", "operator %="},
                                                  {"opPowAssign", "operator **="},
                                                  {"opAndAssign", "operator &="},
                                                  {"opOrAssign",  "operator |="},
                                                  {"opXorAssign", "operator ^="},
                                                  {"opShlAssign", "operator <<="},
                                                  {"opShrAssign", "operator >>="},

                                                  {"opAdd", "operator +"},
                                                  {"opSub", "operator -"},
                                                  {"opMul", "operator *"},
                                                  {"opDiv", "operator /"},
                                                  {"opMod", "operator %"},
                                                  {"opAnd", "operator &"},
                                                  {"opOr",  "operator |"},
                                                  {"opXor", "operator ^"},
                                                  {"opShl",  "operator <<"},
                                                  {"opShr", "operator >>"},

                                                  {"opEquals", "opeartor =="},
                                                  {"opCmp", "opeartor <"},

                                                  {"opIndex", "opeartor []"}
                                                };

    AngelClassItem *classItem = new AngelClassItem({info->GetName(),
                                                    itemIcons.value(type)}, m_pRootItem);
    m_pRootItem->appendChild(classItem);

    QList<QPair<QString, QString>> nativeProperties;

    const QImage &methodIcon = itemIcons.value(AngelItem::Method);
    for(uint32_t m = 0; m < info->GetMethodCount(); m++) {
        asIScriptFunction *method = info->GetMethodByIndex(m);
        if(method) {
            bool drop = false;
            QString name(method->GetName());
            for(auto it : dropList) {
                if(name.contains(it)) {
                    drop = true;
                    break;
                }
            }

            if(name.contains(GET)) {
                drop = true;
                int32_t typeId = method->GetReturnTypeId();
                asITypeInfo *type = m_pEngine->GetTypeInfoById(typeId);
                QPair<QString, QString> pair;
                pair.first = name.replace(GET, "");
                if(type) {
                    pair.second = type->GetName();
                } else {
                    pair.second = baseTypes.value(static_cast<asETypeIdFlags>(typeId), "void");
                }
                nativeProperties.append(pair);
            }

            if(!drop) {
                name = replaceMap.value(name, name);
                classItem->appendChild(new AngelClassItem({name, methodIcon, exportParams(method)}, classItem));
            }
        }
    }

    const QImage &propertyIcon = itemIcons.value(AngelItem::Property);
    for(uint32_t p = 0; p < info->GetPropertyCount(); p++) {
        const char *name;
        int32_t typeId;
        bool isPrivate;
        info->GetProperty(p, &name, &typeId, &isPrivate);

        QString type;
        asITypeInfo *param = m_pEngine->GetTypeInfoById(typeId);
        if(param) {
            type = param->GetName();
        } else {
            type = baseTypes.value(static_cast<asETypeIdFlags>(typeId), "void");
        }

        if(!isPrivate) {
            classItem->appendChild(new AngelClassItem({name, propertyIcon, type}, classItem));
        }
    }

    for(auto it : nativeProperties) {
        classItem->appendChild(new AngelClassItem({it.first, propertyIcon, it.second}, classItem));
    }

    const QImage &enumIcon = itemIcons.value(AngelItem::Enum);
    for(uint32_t e = 0; e < info->GetEnumValueCount(); e++) {
        classItem->appendChild(new AngelClassItem({info->GetEnumValueByIndex(e, nullptr), enumIcon}, classItem));
    }
}

QStringList AngelClassMapModel::exportParams(asIScriptFunction *method) {
    QStringList params;
    for(uint32_t p = 0; p < method->GetParamCount(); p++) {
        int32_t typeId;
        asDWORD flags;
        method->GetParam(p, &typeId, &flags);

        asITypeInfo *param = m_pEngine->GetTypeInfoById(typeId);
        if(param) {
            params.push_back(param->GetName());
        } else {
            params.push_back(baseTypes.value(static_cast<asETypeIdFlags>(typeId), "void"));
        }
    }
    return params;
}

int AngelClassMapModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 1;
}

int AngelClassMapModel::rowCount(const QModelIndex &parent) const {
    AngelClassItem *parentItem;
    if(parent.column() > 0) {
        return 0;
    }

    if(!parent.isValid()) {
        parentItem = m_pRootItem;
    } else {
        parentItem = static_cast<AngelClassItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

QModelIndex AngelClassMapModel::index(int row, int column, const QModelIndex &parent) const {
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    AngelClassItem *parentItem;
    if(!parent.isValid()) {
        parentItem = m_pRootItem;
    } else {
        parentItem = static_cast<AngelClassItem*>(parent.internalPointer());
    }

    AngelClassItem *childItem = parentItem->child(row);
    if(childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex AngelClassMapModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    AngelClassItem *childItem = static_cast<AngelClassItem*>(index.internalPointer());
    AngelClassItem *parentItem = childItem->parentItem();

    if(parentItem == m_pRootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant AngelClassMapModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    AngelClassItem *item = static_cast<AngelClassItem *>(index.internalPointer());
    if(item) {
        switch(role) {
            case Qt::DisplayRole: {
                QString result = item->data(0).toString();

                QVariant sub = item->data(2);
                if(sub.isValid()) {
                    if(sub.userType() == QMetaType::QStringList) {
                        result += " (" + sub.toStringList().join(", ") + ")";
                    } else {
                        result += " " + sub.toString();
                    }
                }

                return result;
            }
            case Qt::DecorationRole: {
                return item->data(1);
            }
            default: break;
        }
    }

    return QVariant();
}


AngelClassItem::AngelClassItem(const QVector<QVariant> &data, AngelClassItem *parent) :
        m_itemData(data),
        m_parentItem(parent) {

}

AngelClassItem::~AngelClassItem() {
    qDeleteAll(m_childItems);
}

void AngelClassItem::appendChild(AngelClassItem *item) {
    m_childItems.append(item);
}

AngelClassItem *AngelClassItem::child(int row) {
    if(row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
}

int AngelClassItem::row() const {
    if(m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<AngelClassItem *>(this));
    }
    return 0;
}

AngelClassItem *AngelClassItem::parentItem() {
    return m_parentItem;
}

QVariant AngelClassItem::data(int column) const {
    if(column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

int AngelClassItem::childCount() const {
    return m_childItems.count();
}
