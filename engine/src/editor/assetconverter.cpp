#include "editor/assetconverter.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QFile>
#include <QCryptographicHash>
#include <QUuid>
// Icon related
#include <QDomDocument>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

#include "editor/projectsettings.h"

#include "config.h"

namespace {
    const char *gMd5("md5");
    const char *gVersion("version");
    const char *gGUID("guid");
    const char *gTemplateName("${templateName}");
};

/*!
    \class AssetConverterSettings
    \brief The AssetConverterSettings class provides configuration and state management for asset conversion processes.
    \inmodule Editor

    The AssetConverterSettings class provides configuration and state management for asset conversion processes.
    It handles metadata storage, version control, and file management for converted assets.
*/

AssetConverterSettings::AssetConverterSettings() :
        m_valid(false),
        m_modified(false),
        m_dir(false),
        m_type(MetaType::INVALID),
        m_version(0),
        m_currentVersion(0) {

    connect(this, &AssetConverterSettings::updated, this, &AssetConverterSettings::setModified);
}

AssetConverterSettings::~AssetConverterSettings() {

}
/*!
    Returns the asset type for conversion for more details see MetaType.
*/
uint32_t AssetConverterSettings::type() const {
    return m_type;
}
/*!
    Sets the asset type for conversion for more details see MetaType.
*/
void AssetConverterSettings::setType(uint32_t type) {
    m_type = type;
}
/*!
    Returns true if asset cannot be cahnged using any embedded editor; returns true by the default.
*/
bool AssetConverterSettings::isReadOnly() const {
    return true;
}
/*!
    Returns true if the asset needs to be reimported; otherwise returns false.
    This method compares asset version, file md5 checksum or file existance.
*/
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
/*!
    Returns whether this asset represents code (default returns false).
*/
bool AssetConverterSettings::isCode() const {
    return false;
}
/*!
    Returns true if asset represents directory; otherwise returns false.
*/
bool AssetConverterSettings::isDir() const {
    return m_dir;
}
/*!
    Returns list of type names for this asset (default returns "Invalid" if type is invalid).
*/
QStringList AssetConverterSettings::typeNames() const {
    if(m_type != MetaType::INVALID) {
        QString result = MetaType::name(m_type);
        result = result.replace("*", "");
        return { result.trimmed() };
    }
    return { "Invalid" };
}
/*!
    Returns primary type name (first from typeNames()).
*/
QString AssetConverterSettings::typeName() const {
    return typeNames().constFirst();
}
void AssetConverterSettings::resetIcon(const QString &uuid) {
    for(auto &it : m_subItems) {
        if(it.uuid == uuid) {
            it.icon = QImage();
            return;
        }
    }

    m_icon = QImage();
}
/*!
    Returns icon assotiatited with current asset \a uuid.
*/
QImage AssetConverterSettings::icon(const QString &uuid) {
    QString path(ProjectSettings::instance()->iconPath() + "/" + uuid + ".png");

    for(auto &it : m_subItems) {
        if(it.uuid == uuid) {
            if(it.icon.isNull() && !it.icon.load(path)) {
                it.icon = documentIcon(MetaType::name(it.typeId));
            }
            return it.icon;
        }
    }

    if(m_icon.isNull() && !m_icon.load(path)) {
        m_icon = documentIcon(typeName());
    }
    return m_icon;
}
/*!
    Returns path to default icon for asset \a type (default returns ":/Style/styles/dark/images/unknown.svg").
*/
QString AssetConverterSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/unknown.svg";
}
/*!
    Returns the md5 checksum of the source file (formatted as a GUID-like string).
*/
QString AssetConverterSettings::hash() const {
    return m_md5;
}
/*!
    Sets the md5 checksum \a hash of the source file (formatted as a GUID-like string).
*/
void AssetConverterSettings::setHash(const QString &hash) {
    m_md5 = hash;
}
/*!
    Returns the asset converter asset format version.
*/
uint32_t AssetConverterSettings::version() const {
    return m_version;
}
/*!
    Sets the asset converter asset format \a version.
*/
void AssetConverterSettings::setVersion(uint32_t version) {
    m_version = version;
}

QImage AssetConverterSettings::renderDocumentIcon(const QString &type, const QString &color) {
    QFile file(":/Style/styles/dark/images/document.svg");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray documentSvg = file.readAll();
        file.close();

        QString path = defaultIconPath(type);

        // Add icon
        QDomDocument doc;
        QFile icon(path);
        if(icon.open(QIODevice::ReadOnly)) {
            doc.setContent(&icon);
            icon.close();

            QDomElement svg = doc.firstChildElement("svg");
            QDomElement defs = svg.firstChildElement("defs");
            QDomElement style = defs.firstChildElement("style");

            documentSvg.replace("{style}", qPrintable(style.text()));

            QString str;
            QTextStream stream(&str);

            QDomElement content = defs.nextSiblingElement();
            while(!content.isNull()) {
                content.save(stream, 4);
                content = content.nextSiblingElement();
            }

            documentSvg.replace("{icon}", qPrintable(str));
        }

        documentSvg.replace("{text}", qPrintable(QFileInfo(path).baseName().toLower()));
        documentSvg.replace("#f0f", qPrintable(color));

        QSvgRenderer renderer;
        renderer.load(documentSvg);

        QImage document(128, 128, QImage::Format_ARGB32);
        document.fill(Qt::transparent);

        QPainter painter(&document);
        renderer.render(&painter);

        return document;
    }

    return QImage();
}
/*!
    Returns the current asset format version.
*/
uint32_t AssetConverterSettings::currentVersion() const {
    return m_currentVersion;
}
/*!
    Sets the current asset format \a version.
*/
void AssetConverterSettings::setCurrentVersion(uint32_t version) {
    m_currentVersion = version;
}
/*!
    Returns the source file path.
*/
QString AssetConverterSettings::source() const {
    return m_source;
}
/*!
    Sets the \a source file path.
*/
void AssetConverterSettings::setSource(const QString &source) {
    m_source = source;
}
/*!
    Returns the destination file path (relative).
*/
QString AssetConverterSettings::destination() const {
    return m_destination;
}
/*!
    Sets the \a destination file path (relative).
*/
void AssetConverterSettings::setDestination(const QString &destination) {
    m_destination = destination;
}
/*!
    Returns the absolute destination file path.
*/
QString AssetConverterSettings::absoluteDestination() const {
    return m_absoluteDestination;
}
/*!
    Sets the absolute \a destination file path.
*/
void AssetConverterSettings::setAbsoluteDestination(const QString &destination) {
    m_absoluteDestination = destination;
}
/*!
    Returns list of all sub-item keys.
*/
const QStringList AssetConverterSettings::subKeys() const {
    return m_subItems.keys();
}
/*!
    Returns UUID of a sub-item by \a key.
*/
QString AssetConverterSettings::subItem(const QString &key) const {
    return m_subItems.value(key).uuid;
}
/*!
    Returns additional data for a sub-item by it's \a key (default returns empty object).
*/
QJsonObject AssetConverterSettings::subItemData(const QString &key) const {
    Q_UNUSED(key)
    return QJsonObject();
}
/*!
    Returns the type name of a sub-item by it's \a key.
*/
QString AssetConverterSettings::subTypeName(const QString &key) const {
    QString result = MetaType::name(subType(key));
    result = result.replace("*", "");
    return result.trimmed();
}
/*!
    Returns the type ID of a sub-item by it's \a key.
*/
int32_t AssetConverterSettings::subType(const QString &key) const {
    return m_subItems.value(key).typeId;
}
/*!
    Marks all sub-items as dirty (modified).
*/
void AssetConverterSettings::setSubItemsDirty() {
    for(auto &it : m_subItems) {
        it.dirty = true;
    }
}
/*!
    Sets a sub-item with \a name, \a uuid, and \a type.
*/
void AssetConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        m_subItems[name] = {uuid, QImage(), type, false};
    }
}
/*!
    Sets additional \a data for a sub-item with given \a name (default does nothing).
*/
void AssetConverterSettings::setSubItemData(const QString &name, const QJsonObject &data) {
    Q_UNUSED(name)
    Q_UNUSED(data)
}
/*!
    Saves binary \a data as a sub-item with given destination \a path and asset \a type.
    This method generated UUID id needed and registers a new sub-item.
    \sa AssetConverterSettings::setSubItem()
*/
QString AssetConverterSettings::saveSubData(const ByteArray &data, const QString &path, int32_t type) {
    QString uuid = subItem(path);
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
    }

    QFileInfo dst(absoluteDestination());
    QFile file(dst.absolutePath() + "/" + uuid);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        setSubItem(path, uuid, type);
    }
    return uuid;
}
/*!
    Loads settings from metadata file ([source].set)
    Each asset in the Conent directory has [source].set file wich contains all meta information and import setting for the asset.
*/
bool AssetConverterSettings::loadSettings() {
    QFile meta(source() + "." + gMetaExt);
    if(meta.open(QIODevice::ReadOnly)) {
        QJsonObject object = QJsonDocument::fromJson(meta.readAll()).object();
        meta.close();

        blockSignals(true);

        const QMetaObject *meta = metaObject();
        QVariantMap map = object.value(gSettings).toObject().toVariantMap();
        for(int32_t index = 0; index < meta->propertyCount(); index++) {
            QMetaProperty property = meta->property(index);
            QVariant v = map.value(property.name(), property.read(this));
            v.convert(property.userType());
            property.write(this, v);
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
/*!
    Saves current import settings to metadata file to ([source].set) file.
    Serializes properties via reflection, version and hash information and stores sub items information in JSON format.
*/
void AssetConverterSettings::saveSettings() {
    QJsonObject set;
    const QMetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        if(QString(property.name()) != "objectName") {
            set.insert(property.name(), QJsonValue::fromVariant(property.read(this)));
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

    QFile fp(QString(source()) + "." + gMetaExt);
    if(fp.open(QIODevice::WriteOnly)) {
        fp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        fp.close();
        m_modified = false;
    }
}
/*!
    Returns true if import setting has been modified; otherwise return false.
*/
bool AssetConverterSettings::isModified() const {
    return m_modified;
}
/*!
    Marks the asset as modified.
    This allows to user decide to save setting and re-import the asset or revert the settings back.
*/
void AssetConverterSettings::setModified() {
    m_modified = true;
}
/*!
    Marks the asset as directory.
*/
void AssetConverterSettings::setDirectory() {
    m_dir = true;
}
/*!
    Returns icon associated with document \a type.
*/
QImage AssetConverterSettings::documentIcon(const QString &type) {
    static std::map<QString, QImage> documents;

    auto it = documents.find(type);
    if(it != documents.end()) {
        return it->second;
    }

    QImage result = renderDocumentIcon(type);
    if(!result.isNull()) {
        documents[type] = result;
    }

    return result;
}

/*!
    \class AssetConverter
    \brief The AssetConverter class is an abstract base class that provides an interface for converting assets in the engine.
    \inmodule Editor

    The AssetConverter class is an abstract base class that provides an interface for converting assets in the engine.
    It's designed to be subclassed for specific asset types that require conversion or processing.

    Example:
    \code
        class TextureConverter : public AssetConverter {
        public:
            QStringList suffixes() const override {
                return {"png", "jpg", "tga"};
            }

            ReturnCode convertFile(AssetConverterSettings *settings) override {
                // Conversion logic here
                return Success;
            }

            AssetConverterSettings *createSettings() override {
                return new TextureSettings();
            }
        };
    \endcode
*/

/*!
    \enum AssetConverter::ReturnCode

    \value Success \c Conversion completed successfully
    \value InternalError \c An unexpected error occurred during conversion
    \value Unsupported \c The asset type is not supported by this converter
    \value Skipped \c Conversion was intentionally skipped
    \value CopyAsIs \c Asset was copied without conversion
*/

/*!
    \fn QStringList AssetConverter::suffixes() const
    Returns the list of file suffixes (extensions) this converter supports (e.g., ["png", "jpg"] for an image converter).
*/
/*!
    \fn ReturnCode AssetConverter::convertFile(AssetConverterSettings *settings)
    Converts a file using the provided settings.
*/

/*!
    Initializes the converter. Can be overridden to perform any necessary setup.
*/
void AssetConverter::init() {

}
/*!
    Creates a new settings object appropriate for this converter type.
*/
AssetConverterSettings *AssetConverter::createSettings() {
    return new AssetConverterSettings();
}
/*!
    Handles asset renaming to react on asset change the \a oldName to \a newName (e.g. for code renaming) for the particaular asset \a settings.
*/
void AssetConverter::renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName) {
    Q_UNUSED(settings)
    Q_UNUSED(oldName)
    Q_UNUSED(newName)
}
/*!
    Creates a new asset file from template and copies it by \a destination path.
*/
void AssetConverter::createFromTemplate(const QString &destination) {
    QFile file(templatePath());
    if(file.open(QFile::ReadOnly)) {
        QByteArray data(file.readAll());
        file.close();

        data.replace(gTemplateName, qPrintable(QFileInfo(destination).baseName()));

        QFile gen(destination);
        if(gen.open(QFile::ReadWrite)) {
            gen.write(data);
            gen.close();
        }
    }
}
/*!
    Returns the path to a template file for creating new assets of this type.
*/
QString AssetConverter::templatePath() const {
    return QString();
}
/*!
    Returns the path to an icon representing this asset type.
*/
QString AssetConverter::iconPath() const {
    return QString();
}
/*!
    Creates an actor with appropriate component using this asset using \a settings and asset \a guid.
*/
Actor *AssetConverter::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    Q_UNUSED(settings)
    Q_UNUSED(guid)
    return nullptr;
}
