#include "editor/assetconverter.h"

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

AssetConverterSettings::AssetConverterSettings() :
        m_Valid(false),
        m_Modified(false),
        m_Type(MetaType::INVALID),
        m_Version(0),
        m_CurrentVersion(0) {

    connect(this, &AssetConverterSettings::updated, this, &AssetConverterSettings::setModified);
}

AssetConverterSettings::~AssetConverterSettings() {

}

uint32_t AssetConverterSettings::type() const {
    return m_Type;
}
void AssetConverterSettings::setType(uint32_t type) {
    m_Type = type;
}

bool AssetConverterSettings::isValid() const {
    return m_Valid;
}
void AssetConverterSettings::setValid(bool valid) {
    m_Valid = valid;
}

bool AssetConverterSettings::isReadOnly() const {
    return true;
}

QString AssetConverterSettings::typeName() const {
    if(m_Type != MetaType::INVALID) {
        QString result = MetaType::name(m_Type);
        result = result.replace("*", "");
        return result.trimmed();
    }
    return "Invalid";
}

QString AssetConverterSettings::hash() const {
    return m_Md5;
}
void AssetConverterSettings::setHash(const QString &hash) {
    m_Md5 = hash;
}

uint32_t AssetConverterSettings::version() const {
    return m_Version;
}

void AssetConverterSettings::setVersion(uint32_t version) {
    m_Version = version;
}

uint32_t AssetConverterSettings::currentVersion() const {
    return m_CurrentVersion;
}

void AssetConverterSettings::setCurrentVersion(uint32_t version) {
    m_CurrentVersion = version;
}

QString AssetConverterSettings::source() const {
    return m_Source;
}
void AssetConverterSettings::setSource(const QString &source) {
    m_Source = source;
}

QString AssetConverterSettings::destination() const {
    return m_Destination;
}
void AssetConverterSettings::setDestination(const QString &destination) {
    m_Destination = destination;
}

QString AssetConverterSettings::absoluteDestination() const {
    return m_AbsoluteDestination;
}

void AssetConverterSettings::setAbsoluteDestination(const QString &destination) {
    m_AbsoluteDestination = destination;
}

const QStringList AssetConverterSettings::subKeys() const {
    return m_SubItems.keys();
}

QString AssetConverterSettings::subItem(const QString &key) const {
    return m_SubItems.value(key);
}

QJsonObject AssetConverterSettings::subItemData(const QString &key) const {
    Q_UNUSED(key)
    return QJsonObject();
}

QString AssetConverterSettings::subTypeName(const QString &key) const {
    QString result = MetaType::name(subType(key));
    result = result.replace("*", "");
    return result.trimmed();
}

int32_t AssetConverterSettings::subType(const QString &key) const {
    return m_SubTypes.value(key);
}

void AssetConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        m_SubItems[name] = uuid;
        m_SubTypes[name] = type;
    }
}

void AssetConverterSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    Q_UNUSED(name)
    Q_UNUSED(data)
}

bool AssetConverterSettings::loadSettings() {
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

void AssetConverterSettings::saveSettings() {
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

bool AssetConverterSettings::isModified() const {
    return m_Modified;
}

void AssetConverterSettings::setModified() {
    m_Modified = true;
}

void AssetConverter::init() {

}

AssetConverterSettings *AssetConverter::createSettings() const {
    return new AssetConverterSettings();
}

QString AssetConverter::templatePath() const {
    return QString();
}

QString AssetConverter::iconPath() const {
    return QString();
}

Actor *AssetConverter::createActor(const QString &guid) const {
    Q_UNUSED(guid)
    return nullptr;
}
