#include "tiledmapconverter.h"

#include <QFileInfo>
#include <QDir>

#include <cstring>

#include <bson.h>

#include <components/actor.h>
#include <components/tilemaprender.h>

#include <resources/sprite.h>
#include <resources/prefab.h>

#include <editor/projectsettings.h>

#define FORMAT_VERSION 1

const char *gTileMapRender("TileMapRender");

TiledMapConverterSettings::TiledMapConverterSettings() {
    setType(MetaType::type<Prefab *>());
    setVersion(FORMAT_VERSION);
}

TString TiledMapConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/tilemap.svg";
}

AssetConverter::ReturnCode TiledMapConverter::convertFile(AssetConverterSettings *settings) {
    QFile file(settings->source().data());
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray buffer(file.readAll());
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
                    root = Engine::objectCreate<Actor>("TileMap", nullptr);
                    root->addComponent("Transform");
                }

                std::list<Component *> components = root->componentsInChild(gTileMapRender);
                std::list<Component *> usedComponents;

                pugi::xml_node element = ts.first_child();

                TileSet *tileSet = nullptr;
                int tileOffset = 0;

                while(element) {
                    if(std::string(element.name()) == "tileset") {
                        TString source(element.attribute("source").as_string());
                        QFileInfo info(settings->source().data());
                        QDir dir(ProjectSettings::instance()->contentPath().data());
                        if(source.isEmpty()) {
                            tileSet = Engine::objectCreate<TileSet>();
                            parseTileset(element, info.path(), *tileSet);

                            TString uuid = settings->saveSubData(Bson::save(Engine::toVariant(tileSet)),
                                                                 element.attribute("name").as_string(), MetaType::type<TileSet *>());

                            TileSet *set = Engine::loadResource<TileSet>(uuid);
                            if(set == nullptr) {
                                Engine::setResource(tileSet, uuid);
                            } else {
                                tileSet = set;
                            }
                        } else {
                            source = dir.relativeFilePath(info.path() + "/" + source.data()).toStdString();

                            tileSet = Engine::loadResource<TileSet>(source);
                        }

                        tileOffset = element.attribute("firstgid").as_int();
                    } else if(std::string(element.name()) == "layer") {
                        TileMap *tileMap = new TileMap;
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

                        TString uuid = settings->saveSubData(Bson::save(Engine::toVariant(tileMap)),
                                                             element.attribute("name").as_string(), MetaType::type<TileMap *>());

                        TileMap *map = Engine::loadResource<TileMap>(uuid);
                        if(map == nullptr) {
                            Engine::setResource(tileSet, uuid);
                        } else {
                            tileMap = map;
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
                            render = static_cast<TileMapRender *>(actor->component(gTileMapRender));
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

                if(prefab == nullptr) {
                    prefab = Engine::objectCreate<Prefab>("");
                    prefab->setActor(root);
                }

                QFile file(settings->absoluteDestination().data());
                if(file.open(QIODevice::WriteOnly)) {
                    ByteArray data = Bson::save(Engine::toVariant(prefab));
                    file.write(reinterpret_cast<const char *>(data.data()), data.size());
                    file.close();

                    return Success;
                }
            }
        }
    }

    return InternalError;
}

void TiledMapConverter::parseTileset(const pugi::xml_node &parent, const QString &path, TileSet &tileSet) {
    tileSet.setTileWidth(parent.attribute("tilewidth").as_int());
    tileSet.setTileHeight(parent.attribute("tileheight").as_int());

    tileSet.setTileSpacing(parent.attribute("spacing").as_int());
    tileSet.setTileMargin(parent.attribute("margin").as_int());

    tileSet.setColumns(parent.attribute("columns").as_int());

    pugi::xml_node element = parent.first_child();
    while(element) {
        if(std::string(element.name()) == "image") {
            QDir dir(ProjectSettings::instance()->contentPath().data());
            TString source(dir.relativeFilePath(path + "/" + element.attribute("source").as_string()).toStdString());

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
        Actor *object = Engine::composeActor("TileMapRender", "");
        TileMapRender *render = object->getComponent<TileMapRender>();
        if(render) {
            render->setTileMap(static_cast<TileMap *>(resource));
        }
        return object;
    }
    return AssetConverter::createActor(settings, guid);
}

