#include "editor/assetconverter.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QFile>
#include <QCryptographicHash>
#include <QUuid>

#include "editor/projectsettings.h"

#include "config.h"

namespace {
    const char *gMd5("md5");
    const char *gVersion("version");
    const char *gGUID("guid");
};

AssetConverterSettings::AssetConverterSettings() :
        m_valid(false),
        m_modified(false),
        m_type(MetaType::INVALID),
        m_version(0),
        m_currentVersion(0) {

    connect(this, &AssetConverterSettings::updated, this, &AssetConverterSettings::setModified);
}

AssetConverterSettings::~AssetConverterSettings() {

}

uint32_t AssetConverterSettings::type() const {
    return m_type;
}
void AssetConverterSettings::setType(uint32_t type) {
    m_type = type;
}

bool AssetConverterSettings::isValid() const {
    return m_valid;
}
void AssetConverterSettings::setValid(bool valid) {
    m_valid = valid;
}

bool AssetConverterSettings::isReadOnly() const {
    return true;
}

bool AssetConverterSettings::isOutdated() const {
    if(version() > currentVersion()) {
        return true;
    }
    bool result = true;

    QFile file(source());
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();

        md5 = md5.insert(20, '-');
        md5 = md5.insert(16, '-');
        md5 = md5.insert(12, '-');
        md5 = md5.insert( 8, '-');
        md5.push_front('{');
        md5.push_back('}');

        if(hash() == md5) {
            if(isCode() || QFileInfo::exists(absoluteDestination())) {
                result = false;
            }
        }
        m_md5 = md5;
    }
    return result;
}

bool AssetConverterSettings::isCode() const {
    return false;
}

QStringList AssetConverterSettings::typeNames() const {
    if(m_type != MetaType::INVALID) {
        QString result = MetaType::name(m_type);
        result = result.replace("*", "");
        return { result.trimmed() };
    }
    return { "Invalid" };
}

QString AssetConverterSettings::typeName() const {
    return typeNames().constFirst();
}

QString AssetConverterSettings::defaultIcon(QString type) const {
    return ":/Style/styles/dark/images/unknown.svg";
}

QString AssetConverterSettings::hash() const {
    return m_md5;
}
void AssetConverterSettings::setHash(const QString &hash) {
    m_md5 = hash;
}

uint32_t AssetConverterSettings::version() const {
    return m_version;
}

void AssetConverterSettings::setVersion(uint32_t version) {
    m_version = version;
}

uint32_t AssetConverterSettings::currentVersion() const {
    return m_currentVersion;
}

void AssetConverterSettings::setCurrentVersion(uint32_t version) {
    m_currentVersion = version;
}

QString AssetConverterSettings::source() const {
    return m_source;
}
void AssetConverterSettings::setSource(const QString &source) {
    m_source = source;
}

QString AssetConverterSettings::destination() const {
    return m_destination;
}
void AssetConverterSettings::setDestination(const QString &destination) {
    m_destination = destination;
}

QString AssetConverterSettings::absoluteDestination() const {
    return m_absoluteDestination;
}

void AssetConverterSettings::setAbsoluteDestination(const QString &destination) {
    m_absoluteDestination = destination;
}

const QStringList AssetConverterSettings::subKeys() const {
    return m_subItems.keys();
}

QString AssetConverterSettings::subItem(const QString &key) const {
    return m_subItems.value(key).uuid;
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
    return m_subItems.value(key).typeId;
}

void AssetConverterSettings::setSubItemsDirty() {
    for(auto &it : m_subItems) {
        it.dirty = true;
    }
}

void AssetConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        m_subItems[name] = {uuid, type, false};
    }
}

void AssetConverterSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    Q_UNUSED(name)
    Q_UNUSED(data)
}

QString AssetConverterSettings::saveSubData(const ByteArray &data, const QString &path, int32_t type) {
    QString uuid = subItem(path);
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
    }

    QFileInfo dst(absoluteDestination());
    QFile file(dst.absolutePath() + "/" + uuid);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(&data[0]), data.size());
        file.close();

        setSubItem(path, uuid, type);
    }
    return uuid;
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
                if(!data.isEmpty()) {
                    setSubItemData(it, data);
                }
            }
        }
        blockSignals(false);
        m_modified = false;
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
        SubItem item = m_subItems.value(it);

        if(!item.dirty) {
            QJsonArray array;
            array.push_back(item.uuid);
            array.push_back(item.typeId);

            QJsonObject data = subItemData(it);
            if(!data.isEmpty()) {
                array.push_back(data);
            }

            array.push_back(subTypeName(it));

            sub[it] = array;
        }
    }
    obj.insert(gSubItems, sub);

    QFile fp(QString(source()) + gMetaExt);
    if(fp.open(QIODevice::WriteOnly)) {
        fp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        fp.close();
        m_modified = false;
    }
}

bool AssetConverterSettings::isModified() const {
    return m_modified;
}

void AssetConverterSettings::setModified() {
    m_modified = true;
}

void AssetConverter::init() {

}

AssetConverterSettings *AssetConverter::createSettings() {
    return new AssetConverterSettings();
}

void AssetConverter::renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName) {
    Q_UNUSED(settings)
    Q_UNUSED(oldName)
    Q_UNUSED(newName)
}

QString AssetConverter::templatePath() const {
    return QString();
}

QString AssetConverter::iconPath() const {
    return QString();
}

Actor *AssetConverter::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    Q_UNUSED(settings)
    Q_UNUSED(guid)
    return nullptr;
}
