#include "tiledmapconverter.h"

#include <QDir>

#include <cstring>

#include <bson.h>
#include <file.h>
#include <url.h>

#include <components/actor.h>
#include <components/tilemaprender.h>

#include <resources/sprite.h>
#include <resources/prefab.h>

#include <editor/projectsettings.h>

#define FORMAT_VERSION 1

const char *gTileMapRender("TileMapRender");

TiledMapConverterSettings::TiledMapConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList TiledMapConverterSettings::typeNames() const {
    return { MetaType::name<Prefab>() };
}

void TiledMapConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/tilemap.svg");
    }
}

AssetConverter::ReturnCode TiledMapConverter::convertFile(AssetConverterSettings *settings) {
    File file(settings->source());
    if(file.open(File::ReadOnly)) {
        TString buffer(file.readAll());
        file.close();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

        if(result) {
            pugi::xml_node ts = doc.document_element();
            if(std::string(ts.name()) == "map") {
                Actor *root = nullptr;
                Prefab *prefab = Engine::loadResource<Prefab>(settings->destination());
                if(prefab) {
                    root = prefab->actor();
                } else {
                    prefab = Engine::objectCreate<Prefab>(settings->destination());

                    root = Engine::composeActor("", "TileMap", nullptr);
                    prefab->setActor(root);
                }

                uint32_t uuid = settings->info().id;
                if(uuid == 0) {
                    uuid = Engine::generateUUID();
                    settings->info().id = uuid;
                }

                if(prefab->uuid() != uuid) {
                    Engine::replaceUUID(prefab, uuid);
                }

                std::list<Component *> components = root->componentsInChild(gTileMapRender);
                std::list<Component *> usedComponents;

                pugi::xml_node element = ts.first_child();

                TileSet *tileSet = nullptr;
                int tileOffset = 0;

                while(element) {
                    if(std::string(element.name()) == "tileset") {
                        TString source(element.attribute("source").as_string());
                        Url info(settings->source());

                        if(source.isEmpty()) {
                            TString tilesetName(element.attribute("name").as_string());
                            ResourceSystem::ResourceInfo resInfo = settings->subItem(tilesetName, true);

                            tileSet = Engine::loadResource<TileSet>(resInfo.uuid);
                            if(tileSet == nullptr) {
                                tileSet = Engine::objectCreate<TileSet>(resInfo.uuid);
                            }

                            parseTileset(element, info.dir(), *tileSet);

                            Url dst(settings->absoluteDestination());

                            AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(tileSet), dst.absoluteDir() + "/" + resInfo.uuid);
                            if(result == AssetConverter::Success) {
                                resInfo.id = tileSet->uuid();
                                resInfo.type = MetaType::name<TileSet>();
                                settings->setSubItem(tilesetName, resInfo);
                            }
                        } else {
                            QDir dir(ProjectSettings::instance()->contentPath().data());
                            source = dir.relativeFilePath((info.dir() + "/" + source).data()).toStdString();

                            tileSet = Engine::loadResource<TileSet>(source);
                        }

                        tileOffset = element.attribute("firstgid").as_int();
                    } else if(std::string(element.name()) == "layer") {
                        TString tilemapName(element.attribute("name").as_string());
                        ResourceSystem::ResourceInfo resInfo = settings->subItem(tilemapName, true);

                        TileMap *tileMap = Engine::loadResource<TileMap>(resInfo.uuid);
                        if(tileMap == nullptr) {
                            tileMap = Engine::objectCreate<TileMap>(resInfo.uuid);
                        }

                        parseLayer(element, tileOffset, *tileMap);
                        tileMap->setTileSet(tileSet);

                        int tileWidth = ts.attribute("tilewidth").as_int();
                        int tileHeight = ts.attribute("tileheight").as_int();

                        TString orientation = ts.attribute("orientation").as_string();
                        if(!orientation.isEmpty()) {
                            TileSet::TileType type = TileSet::Orthogonal;
                            if(orientation == "isometric") {
                                type = TileSet::Isometric;
                            } else if(orientation == "hexagonal") {
                                type = TileSet::Hexagonal;

                                //bool isY = (ts.attribute("staggeraxis") == "y");
                                tileMap->setHexSideLength(ts.attribute("hexsidelength").as_int());
                                tileMap->setHexOdd(std::string(ts.attribute("staggerindex").as_string()) == "odd");
                            }
                            tileMap->setOrientation(type);
                        }

                        tileMap->setCellWidth(tileWidth);
                        tileMap->setCellHeight(tileHeight);

                        Url dst(settings->absoluteDestination());

                        AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(tileMap), dst.absoluteDir() + "/" + resInfo.uuid);
                        if(result == AssetConverter::Success) {
                            resInfo.id = tileMap->uuid();
                            resInfo.type = MetaType::name<TileMap>();
                            settings->setSubItem(tilemapName, resInfo);
                        }

                        TileMapRender *render = nullptr;

                        std::string name = element.attribute("name").as_string();
                        for(auto it : components) {
                            if(it->actor()->name() == name) {
                                render = static_cast<TileMapRender *>(it);
                                usedComponents.push_back(it);
                            }
                        }

                        if(render == nullptr) {
                            Actor *actor = Engine::composeActor(gTileMapRender, name, root);
                            render = actor->getComponent<TileMapRender>();
                        }
                        render->setTileMap(tileMap);
                        render->setLayer(element.attribute("id").as_int());
                    }

                    element = element.next_sibling();
                }

                for(auto it : components) {
                    bool found = false;
                    for(auto used : usedComponents) {
                        if(it == used) {
                            found = true;
                        }
                    }
                    if(!found) {
                        delete it->actor();
                    }
                }

                return settings->saveBinary(Engine::toVariant(prefab), settings->absoluteDestination());
            }
        }
    }

    return InternalError;
}

void TiledMapConverter::parseTileset(const pugi::xml_node &parent, const TString &path, TileSet &tileSet) {
    tileSet.setTileWidth(parent.attribute("tilewidth").as_int());
    tileSet.setTileHeight(parent.attribute("tileheight").as_int());

    tileSet.setTileSpacing(parent.attribute("spacing").as_int());
    tileSet.setTileMargin(parent.attribute("margin").as_int());

    tileSet.setColumns(parent.attribute("columns").as_int());

    pugi::xml_node element = parent.first_child();
    while(element) {
        if(std::string(element.name()) == "image") {
            QDir dir(ProjectSettings::instance()->contentPath().data());
            TString source(dir.relativeFilePath((path + "/" + element.attribute("source").as_string()).data()).toStdString());

            tileSet.setSpriteSheet(Engine::loadResource<Sprite>(source));
        } else if(std::string(element.name()) == "tileoffset") {
            tileSet.setTileOffset(Vector2(element.attribute("x").as_float(),
                                          element.attribute("y").as_float()));
        } else if(std::string(element.name()) == "grid") {
            TileSet::TileType type = TileSet::Orthogonal;
            if(std::string(element.attribute("orientation").as_string()) == "isometric") {
                type = TileSet::Isometric;
            }
            tileSet.setType(type);
        }

        element = element.next_sibling();
    }
}

void TiledMapConverter::parseLayer(const pugi::xml_node &parent, int tileOffset, TileMap &tileMap) {
    uint32_t width = parent.attribute("width").as_uint();
    uint32_t height = parent.attribute("height").as_uint();

    tileMap.setWidth(width);
    tileMap.setHeight(height);

    pugi::xml_node field = parent.first_child();
    while(field) {
        if(std::string(field.name()) == "data") {
            TString encoding(field.attribute("encoding").as_string());
            TString compression(field.attribute("compression").as_string());

            if(encoding == "base64") {
                QByteArray decoded(QByteArray::fromBase64(field.text().as_string()));

                if(compression == "zlib") {
                    int size = width * height * sizeof(int);
                    decoded.push_front(uint8_t(size));
                    decoded.push_front(uint8_t(size >> 8));
                    decoded.push_front(uint8_t(size >> 16));
                    decoded.push_front(uint8_t(size >> 24));

                    decoded = qUncompress(decoded);
                }

                uint32_t *data = reinterpret_cast<uint32_t *>(decoded.data());
                if(data) {
                    std::vector<int> buffer;
                    buffer.resize(decoded.size() / 4);
                    memcpy(buffer.data(), data, decoded.size());

                    for(int x = 0; x < width; x++) {
                        for(int y = 0; y < height; y++) {
                            tileMap.setTile(x, y, buffer[y * width + x] - tileOffset);
                        }
                    }
                }
            }
        }

        field = field.next_sibling();
    }
}

AssetConverterSettings *TiledMapConverter::createSettings() {
    return new TiledMapConverterSettings();
}

Actor *TiledMapConverter::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Resource *resource = Engine::loadResource<Resource>(guid);
    if(dynamic_cast<Prefab *>(resource) != nullptr) {
        return static_cast<Actor *>(static_cast<Prefab *>(resource)->actor()->clone());
    } else if(dynamic_cast<TileMap *>(resource) != nullptr) {
        Actor *object = Engine::composeActor<TileMapRender>("");
        TileMapRender *render = object->getComponent<TileMapRender>();
        if(render) {
            render->setTileMap(static_cast<TileMap *>(resource));
        }
        return object;
    }
    return AssetConverter::createActor(settings, guid);
}

