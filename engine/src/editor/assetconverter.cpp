#include "editor/assetconverter.h"

#include <QFile>
#include <QCryptographicHash>
// Icon related
#include <QtSvg/QSvgRenderer>
#include <QPainter>

#include "editor/projectsettings.h"

#include "config.h"

#include <pugixml.hpp>
#include <sstream>

#include <json.h>
#include <bson.h>
#include <url.h>
#include <file.h>

#include <os/uuid.h>

namespace {
    const char *gMd5("md5");
    const char *gVersion("version");
    const char *gGUID("guid");
    const char *gId("id");
    const char *gMeta("meta");
    const char *gTemplateName("${templateName}");
};

std::map<TString, TString> AssetConverterSettings::m_defaultIcons;

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
        m_version(0),
        m_currentVersion(0),
        m_converter(nullptr) {

}

AssetConverterSettings::~AssetConverterSettings() {

}
/*!
    Returns assotiated AssetConverter.
*/
AssetConverter *AssetConverterSettings::converter() {
    return m_converter;
}
/*!
    Sets assotiated asset \a converter.
*/
void AssetConverterSettings::setConverter(AssetConverter *converter) {
    m_converter = converter;
}
/*!
    Returns the asset type for conversion for more details see MetaType.
*/
uint32_t AssetConverterSettings::type() const {
    return MetaType::type(typeName().data());
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

    QFile file(source().data());
    if(file.open(QFile::ReadOnly)) {
        QByteArray md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();

        md5 = md5.insert(20, '-');
        md5 = md5.insert(16, '-');
        md5 = md5.insert(12, '-');
        md5 = md5.insert( 8, '-');
        md5.push_front('{');
        md5.push_back('}');

        if(hash() == md5.toStdString()) {
            if(isCode() || File::exists(absoluteDestination())) {
                result = false;
            }
        }
        m_info.md5 = md5.toStdString();
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
StringList AssetConverterSettings::typeNames() const {
    return { "Invalid" };
}
/*!
    Returns primary type name (first from typeNames()).
*/
TString AssetConverterSettings::typeName() const {
    return typeNames().front();
}
void AssetConverterSettings::resetIcon(const TString &uuid) {
    for(auto &it : m_subItems) {
        if(it.second.info.uuid == uuid) {
            it.second.icon = QImage();
            return;
        }
    }

    m_icon = QImage();
}
/*!
    Returns icon assotiatited with current asset \a uuid.
*/
QImage AssetConverterSettings::icon(const TString &uuid) {
    TString iconPath(ProjectSettings::instance()->iconPath() + "/" + uuid + ".png");

    for(auto &it : m_subItems) {
        if(it.second.info.uuid == uuid) {
            if(it.second.icon.isNull() && !it.second.icon.load(iconPath.data())) {
                it.second.icon = documentIcon(it.second.info.type);
            }
            return it.second.icon;
        }
    }

    if(m_icon.isNull() && !m_icon.load(iconPath.data())) {
        m_icon = documentIcon(m_suffix);
    }
    return m_icon;
}
/*!
    Returns path to default icon for asset \a type (default returns ":/Style/styles/dark/images/unknown.svg").
*/
TString AssetConverterSettings::defaultIconPath(const TString &type) {
    auto it = m_defaultIcons.find(type);
    if(it != m_defaultIcons.end()) {
        return it->second;
    }

    return ":/Style/styles/dark/images/unknown.svg";
}
/*!
    Adds a new \a path default icon for the asset \a type.
    \note Icon must be in SVG format.
*/
void AssetConverterSettings::setDefaultIconPath(const TString &type, const TString &path) {
    m_defaultIcons[type] = path;
}
/*!
    Returns the md5 checksum of the source file (formatted as a GUID-like string).
*/
TString AssetConverterSettings::hash() const {
    return m_info.md5;
}

ResourceSystem::ResourceInfo &AssetConverterSettings::info() const {
    return m_info;
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

QImage AssetConverterSettings::renderDocumentIcon(const TString &path, const TString &color) {
    QFile file(":/Style/styles/dark/images/document.svg");
    if(file.open(QFile::ReadOnly)) {
        QByteArray documentSvg(file.readAll());
        file.close();

        // Add icon
        QFile icon(path.data());
        if(icon.open(QFile::ReadOnly)) {
            QByteArray buffer(icon.readAll());
            icon.close();

            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

            if(result) {
                pugi::xml_node defs = doc.first_element_by_path("svg/defs");
                pugi::xml_node style = defs.first_element_by_path("style");

                documentSvg.replace("{style}", style.text().as_string());

                std::stringstream ss;

                pugi::xml_node content = defs.next_sibling();
                while(content) {
                    content.print(ss, "", pugi::format_raw);
                    content = content.next_sibling();
                }

                std::string test = ss.str();
                documentSvg.replace("{icon}", ss.str().c_str());
            }
        }

        documentSvg.replace("{text}", Url(path).baseName().toLower().data());
        documentSvg.replace("#f0f", color.data());

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
TString AssetConverterSettings::source() const {
    return m_source;
}
/*!
    Sets the \a source file path.
*/
void AssetConverterSettings::setSource(const TString &source) {
    m_source = source;
    m_suffix = Url(m_source).suffix();
}
/*!
    Returns the destination file path (relative).
*/
TString AssetConverterSettings::destination() const {
    return m_info.uuid;
}
/*!
    Returns the absolute destination file path.
*/
TString AssetConverterSettings::absoluteDestination() const {
    return ProjectSettings::instance()->importPath() + "/" + m_info.uuid;
}
/*!
    \internal
*/
TString AssetConverterSettings::propertyAllias(const TString &name) const {
    return name;
}
/*!
    Returns list of all sub-item keys.
*/
const StringList AssetConverterSettings::subKeys() const {
    StringList result;
    for(auto &it : m_subItems) {
        result.push_back(it.first);
    }
    return result;
}
/*!
    Marks all sub-items as dirty (modified).
*/
void AssetConverterSettings::setSubItemsDirty() {
    for(auto &it : m_subItems) {
        it.second.dirty = true;
    }
}
/*!
    Returns UUID of a sub-item by \a key.
*/
ResourceSystem::ResourceInfo AssetConverterSettings::subItem(const TString &key, bool create) const {
    auto it = m_subItems.find(key);
    if(it != m_subItems.end()) {
        return it->second.info;
    }
    if(create) {
        ResourceSystem::ResourceInfo info;
        info.uuid = Uuid::createUuid().toString();
        info.md5 = m_info.md5;
        return info;
    }
    return ResourceSystem::ResourceInfo();
}
/*!
    Sets a sub-item with \a name, \a info, and \a type.
*/
void AssetConverterSettings::setSubItem(const TString &name, const ResourceSystem::ResourceInfo &info) {
    if(!name.isEmpty() && !info.uuid.isEmpty()) {
        m_subItems[name] = {info, QImage(), false};
    }
}
/*!
    Returns additional data for a sub-item by it's \a key (default returns empty object).
*/
Variant AssetConverterSettings::subItemData(const TString &key) const {
    Q_UNUSED(key)
    return Variant();
}
/*!
    Sets additional \a data for a sub-item with given \a name (default does nothing).
*/
void AssetConverterSettings::setSubItemData(const TString &name, const Variant &data) {
    Q_UNUSED(name)
    Q_UNUSED(data)
}

AssetConverter::ReturnCode AssetConverterSettings::saveBinary(const Variant &data, const TString &path) {
    File file(path);
    if(file.open(File::WriteOnly)) {
        std::set<TString> types;
        for(auto &it : data.toList()) {
            types.insert(it.toList().begin()->toString());
        }

        ProjectSettings::instance()->reportTypes(types);

        file.write(Bson::save(data));
        file.close();

        return AssetConverter::Success;
    }

    return AssetConverter::InternalError;
}
/*!
    Loads settings from metadata file ([source].set)
    Each asset in the Conent directory has [source].set file wich contains all meta information and import setting for the asset.
*/
bool AssetConverterSettings::loadSettings() {
    File meta(source() + "." + gMetaExt);
    if(meta.open(File::ReadOnly)) {
        VariantMap object = Json::load(meta.readAll()).toMap();
        meta.close();

        blockSignals(true);

        const MetaObject *meta = metaObject();
        VariantMap map = object[gSettings].toMap();
        for(int32_t index = 0; index < meta->propertyCount(); index++) {
            MetaProperty property = meta->property(index);
            Variant v = property.read(this);
            auto it = map.find(property.name());
            if(it != map.end()) {
                v = it->second;
            } else {
                it = map.find(propertyAllias(property.name()));
                if(it != map.end()) {
                    v = it->second;
                }
            }
            v.convert(MetaType::type(property.type().name()));
            property.write(this, v);
        }

        auto it = object.find(gGUID);
        if(it != object.end()) {
            m_info.uuid = it->second.toString();
        }

        it = object.find(gMd5);
        if(it != object.end()) {
            m_info.md5 = it->second.toString();
        }

        it = object.find(gType);
        if(it != object.end()) {
            m_info.type = it->second.toString();
        }
        if(m_info.type.isEmpty()) {
            m_info.type = typeName();
        }

        it = object.find(gId);
        if(it != object.end()) {
            m_info.id = static_cast<uint32_t>(it->second.toInt());
        }

        it = object.find(gVersion);
        if(it != object.end()) {
            setCurrentVersion(it->second.toInt());
        }

        it = object.find(gMeta);
        if(it != object.end()) {
            loadUserData(it->second.toMap());
        }

        it = object.find(gSubItems);
        if(it != object.end()) {
            VariantMap sub = it->second.toMap();
            for(auto &subIt : sub) {
                ResourceSystem::ResourceInfo info;
                info.md5 = m_info.md5;

                VariantList array = subIt.second.toList();
                auto item = array.begin();
                info.uuid = item->toString();
                item++;
                if(item->type() == MetaType::INTEGER) {
                    info.type = MetaType::name(item->toInt());
                } else {
                    info.type = item->toString();
                }
                setSubItem(subIt.first, info);
                item++;
                if(item != array.end()) {
                    if(item->userType() == MetaType::INTEGER) {
                        info.id = static_cast<uint32_t>(item->toInt());
                        setSubItem(subIt.first, info); // Update id
                        item++;
                    }

                    if(item != array.end()) {
                        VariantMap data = item->toMap();
                        if(!data.empty()) {
                            setSubItemData(subIt.first, data);
                        }
                    }
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
    VariantMap set;
    const MetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        if(TString(property.name()) != "objectName") {
            set[property.name()] = property.read(this);
        }
    }

    VariantMap obj;
    obj[gVersion] = currentVersion();
    obj[gMd5] = hash();
    obj[gGUID] = destination();
    obj[gSettings] = set;
    obj[gMeta] = saveUserData();
    obj[gId] = m_info.id;
    obj[gType] = m_info.type;

    VariantMap sub;
    for(auto &it : m_subItems) {
        const SubItem &item = it.second;

        if(!item.dirty) {
            VariantList array;
            array.push_back(item.info.uuid);
            array.push_back(item.info.type);
            array.push_back(item.info.id);

            sub[it.first] = array;
        }
    }
    obj[gSubItems] = sub;

    File fp(source() + "." + gMetaExt);
    if(fp.open(File::WriteOnly)) {
        fp.write(Json::save(obj, 0));
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

    updated();
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
QImage AssetConverterSettings::documentIcon(const TString &type) {
    static std::map<TString, QImage> documents;

    auto it = documents.find(type);
    if(it != documents.end()) {
        return it->second;
    }

    QImage result = renderDocumentIcon(defaultIconPath(type));
    if(!result.isNull()) {
        documents[type] = result;
    }

    return result;
}
/*!
    Emmits signal when asset settings has been changed.
*/
void AssetConverterSettings::updated() {
    emitSignal(_SIGNAL(updated()));
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
void AssetConverter::renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName) {
    Q_UNUSED(settings)
    Q_UNUSED(oldName)
    Q_UNUSED(newName)
}
/*!
    Creates a new asset file from template and copies it by \a destination path.
*/
void AssetConverter::createFromTemplate(const TString &destination) {
    QFile file(templatePath().data());
    if(file.open(QFile::ReadOnly)) {
        QByteArray data(file.readAll());
        file.close();

        data.replace(gTemplateName, Url(destination).baseName().data());

        QFile gen(destination.data());
        if(gen.open(QFile::WriteOnly)) {
            gen.write(data);
            gen.close();
        }
    }
}
/*!
    Returns the path to a template file for creating new assets of this type.
*/
TString AssetConverter::templatePath() const {
    return TString();
}
/*!
    Creates an actor with appropriate component using this asset using \a settings and asset \a guid.
*/
Actor *AssetConverter::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Q_UNUSED(settings)
    Q_UNUSED(guid)
    return nullptr;
}
