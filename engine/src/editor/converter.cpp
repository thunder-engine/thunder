#include "editor/converter.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QFile>

#include "config.h"

namespace {
    const QString gMd5("md5");
    const QString gVersion("version");
    const QString gGUID("guid");

    const QString gEntry(".entry");
    const QString gCompany(".company");
    const QString gProject(".project");
};

IConverterSettings::IConverterSettings() :
        m_Valid(false),
        m_Modified(false),
        m_Type(MetaType::INVALID),
        m_Version(0),
        m_CurrentVersion(0) {

    connect(this, &IConverterSettings::updated, this, &IConverterSettings::setModified);
}

IConverterSettings::~IConverterSettings() {

}

uint32_t IConverterSettings::type() const {
    return m_Type;
}
void IConverterSettings::setType(uint32_t type) {
    m_Type = type;
}

bool IConverterSettings::isValid() const {
    return m_Valid;
}
void IConverterSettings::setValid(bool valid) {
    m_Valid = valid;
}

QString IConverterSettings::typeName() const {
    if(m_Type != MetaType::INVALID) {
        QString result = MetaType::name(m_Type);
        result = result.replace("*", "");
        return result.trimmed();
    }
    return "Invalid";
}

QString IConverterSettings::hash() const {
    return m_Md5;
}
void IConverterSettings::setHash(const QString &hash) {
    m_Md5 = hash;
}

uint32_t IConverterSettings::version() const {
    return m_Version;
}

void IConverterSettings::setVersion(uint32_t version) {
    m_Version = version;
}

uint32_t IConverterSettings::currentVersion() const {
    return m_CurrentVersion;
}

void IConverterSettings::setCurrentVersion(uint32_t version) {
    m_CurrentVersion = version;
}

QString IConverterSettings::source() const {
    return m_Source;
}
void IConverterSettings::setSource(const QString &source) {
    m_Source = source;
}

QString IConverterSettings::destination() const {
    return m_Destination;
}
void IConverterSettings::setDestination(const QString &destination) {
    m_Destination = destination;
}

QString IConverterSettings::absoluteDestination() const {
    return m_AbsoluteDestination;
}

void IConverterSettings::setAbsoluteDestination(const QString &destination) {
    m_AbsoluteDestination = destination;
}

const QStringList IConverterSettings::subKeys() const {
    return m_SubItems.keys();
}

QString IConverterSettings::subItem(const QString &key) const {
    return m_SubItems.value(key);
}

QJsonObject IConverterSettings::subItemData(const QString &key) const {
    Q_UNUSED(key)
    return QJsonObject();
}

QString IConverterSettings::subTypeName(const QString &key) const {
    QString result = MetaType::name(subType(key));
    result = result.replace("*", "");
    return result.trimmed();
}

int32_t IConverterSettings::subType(const QString &key) const {
    return m_SubTypes.value(key);
}

void IConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        m_SubItems[name] = uuid;
        m_SubTypes[name] = type;
    }
}

void IConverterSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    Q_UNUSED(name)
    Q_UNUSED(data)
}

bool IConverterSettings::loadSettings() {
    QFile meta(source() + gMetaExt);
    if(meta.open(QIODevice::ReadOnly)) {
        QJsonObject object = QJsonDocument::fromJson(meta.readAll()).object();
        meta.close();

        blockSignals(true);
        QObject *obj = dynamic_cast<QObject *>(this);
        if(obj) {
            const QMetaObject *meta = obj->metaObject();
            QVariantMap map = object.value(gSettings).toObject().toVariantMap();
            for(int32_t index = 0; index < meta->propertyCount(); index++) {
                QMetaProperty property = meta->property(index);
                QVariant v = map.value(property.name(), property.read(obj));
                v.convert(property.userType());
                property.write(obj, v);
            }
        }
        setDestination(object.value(gGUID).toString());
        setHash(object.value(gMd5).toString());
        setCurrentVersion(uint32_t(object.value(gVersion).toInt()));

        QJsonObject sub = object.value(gSubItems).toObject();
        foreach(QString it, sub.keys()) {
            QJsonArray array = sub.value(it).toArray();
            auto item = array.begin();
            QString uuid = item->toString();
            item++;
            int type = item->toInt();
            setSubItem(it, uuid, type);
            item++;
            if(item != array.end()) {
                QJsonObject data = item->toObject();
                setSubItemData(it, data);
            }
        }
        blockSignals(false);
        m_Modified = false;
        return true;
    }
    return false;
}

void IConverterSettings::saveSettings() {
    QJsonObject set;
    QObject *object = dynamic_cast<QObject *>(this);
    if(object) {
        const QMetaObject *meta = object->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property = meta->property(i);
            if(QString(property.name()) != "objectName") {
                set.insert(property.name(), QJsonValue::fromVariant(property.read(object)));
            }
        }
    }

    QJsonObject obj;
    obj.insert(gVersion, int(currentVersion()));
    obj.insert(gMd5, hash());
    obj.insert(gGUID, destination());
    obj.insert(gSettings, set);
    obj.insert(gType, static_cast<int>(type()));

    QJsonObject sub;
    for(const QString &it : subKeys()) {
        QJsonArray array;
        QString uuid = subItem(it);
        array.push_back(uuid);
        array.push_back(subType(it));

        QJsonObject data = subItemData(it);
        if(!data.isEmpty()) {
            array.push_back(data);
        }

        sub[it] = array;
    }
    obj.insert(gSubItems, sub);

    QFile fp(QString(source()) + gMetaExt);
    if(fp.open(QIODevice::WriteOnly)) {
        fp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        fp.close();
        m_Modified = false;
    }
}

bool IConverterSettings::isModified() const {
    return m_Modified;
}

void IConverterSettings::setModified() {
    m_Modified = true;
}

void IConverter::init() {

}

IConverterSettings *IConverter::createSettings() const {
    return new IConverterSettings();
}

QString IConverter::templatePath() const {
    return QString();
}

QString IConverter::iconPath() const {
    return QString();
}

Actor *IConverter::createActor(const QString &guid) const {
    Q_UNUSED(guid)
    return nullptr;
}
