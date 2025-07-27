#include "tiledmapconverter.h"

#include <QFileInfo>
#include <QUuid>
#include <QDir>

#include <QDomDocument>

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

QString TiledMapConverterSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/tilemap.svg";
}

AssetConverter::ReturnCode TiledMapConverter::convertFile(AssetConverterSettings *settings) {
    QFile file(settings->source());
    if(file.open(QIODevice::ReadOnly)) {
        QDomDocument doc;
        if(doc.setContent(&file)) {
            QDomElement ts = doc.documentElement();
            if(ts.nodeName() == "map") {
                Actor *root = nullptr;
                Prefab *prefab = Engine::loadResource<Prefab>(settings->destination().toStdString());
                if(prefab) {
                    root = prefab->actor();
                } else {
                    root = Engine::objectCreate<Actor>("TileMap", nullptr);
                    root->addComponent("Transform");
                }

                std::list<Component *> components = root->componentsInChild(gTileMapRender);
                std::list<Component *> usedComponents;

                QDomNode n = ts.firstChild();

                TileSet *tileSet = nullptr;
                int tileOffset = 0;

                while(!n.isNull()) {
                    QDomElement element = n.toElement();

                    if(element.tagName() == "tileset") {
                        QString source(element.attribute("source"));
                        QFileInfo info(settings->source());
                        QDir dir(ProjectSettings::instance()->contentPath());
                        if(source.isEmpty()) {
                            tileSet = Engine::objectCreate<TileSet>();
                            parseTileset(element, info.path(), *tileSet);

                            QString uuid = settings->saveSubData(Bson::save(Engine::toVariant(tileSet)),
                                                                 element.attribute("name"), MetaType::type<TileSet *>());

                            TileSet *set = Engine::loadResource<TileSet>(uuid.toStdString());
                            if(set == nullptr) {
                                Engine::setResource(tileSet, uuid.toStdString());
                            } else {
                                tileSet = set;
                            }
                        } else {
                            source = dir.relativeFilePath(info.path() + "/" + source);

                            tileSet = Engine::loadResource<TileSet>(source.toStdString());
                        }

                        tileOffset = element.attribute("firstgid").toInt();
                    } else if(element.tagName() == "layer") {
                        TileMap *tileMap = new TileMap;
                        parseLayer(element, tileOffset, *tileMap);
                        tileMap->setTileSet(tileSet);

                        int tileWidth = ts.attribute("tilewidth").toInt();
                        int tileHeight = ts.attribute("tileheight").toInt();

                        QString orientation = ts.attribute("orientation");
                        if(!orientation.isEmpty()) {
                            TileSet::TileType type = TileSet::Orthogonal;
                            if(orientation == "isometric") {
                                type = TileSet::Isometric;
                            } else if(orientation == "hexagonal") {
                                type = TileSet::Hexagonal;

                                //bool isY = (ts.attribute("staggeraxis") == "y");
                                tileMap->setHexSideLength(ts.attribute("hexsidelength").toInt());
                                tileMap->setHexOdd((ts.attribute("staggerindex") == "odd"));
                            }
                            tileMap->setOrientation(type);
                        }

                        tileMap->setCellWidth(tileWidth);
                        tileMap->setCellHeight(tileHeight);

                        QString uuid = settings->saveSubData(Bson::save(Engine::toVariant(tileMap)),
                                                             element.attribute("name"), MetaType::type<TileMap *>());

                        TileMap *map = Engine::loadResource<TileMap>(uuid.toStdString());
                        if(map == nullptr) {
                            Engine::setResource(tileSet, uuid.toStdString());
                        } else {
                            tileMap = map;
                        }

                        TileMapRender *render = nullptr;

                        std::string name = element.attribute("name").toStdString();
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
                        render->setLayer(element.attribute("id").toInt());
                    }

                    n = n.nextSibling();
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

                QFile file(settings->absoluteDestination());
                if(file.open(QIODevice::WriteOnly)) {
                    ByteArray data = Bson::save(Engine::toVariant(prefab));
                    file.write(reinterpret_cast<const char *>(&data[0]), data.size());
                    file.close();

                    return Success;
                }
            }
        }
    }

    return InternalError;
}

void TiledMapConverter::parseTileset(const QDomElement &element, const QString &path, TileSet &tileSet) {
    tileSet.setTileWidth(element.attribute("tilewidth").toInt());
    tileSet.setTileHeight(element.attribute("tileheight").toInt());

    tileSet.setTileSpacing(element.attribute("spacing").toInt());
    tileSet.setTileMargin(element.attribute("margin").toInt());

    tileSet.setColumns(element.attribute("columns").toInt());

    QDomNode n = element.firstChild();
    while(!n.isNull()) {
        QDomElement element = n.toElement();

        if(element.tagName() == "image") {
            QDir dir(ProjectSettings::instance()->contentPath());
            QString source(dir.relativeFilePath(path + "/" + element.attribute("source")));

            tileSet.setSpriteSheet(Engine::loadResource<Sprite>(source.toStdString()));
        } else if(element.tagName() == "tileoffset") {
            tileSet.setTileOffset(Vector2(element.attribute("x").toFloat(),
                                          element.attribute("y").toFloat()));
        } else if(element.tagName() == "grid") {
            TileSet::TileType type = TileSet::Orthogonal;
            if(element.attribute("orientation") == "isometric") {
                type = TileSet::Isometric;
            }
            tileSet.setType(type);
        }

        n = n.nextSibling();
    }
}

void TiledMapConverter::parseLayer(const QDomElement &element, int tileOffset, TileMap &tileMap) {
    uint32_t width = element.attribute("width").toUInt();
    uint32_t height = element.attribute("height").toUInt();

    tileMap.setWidth(width);
    tileMap.setHeight(height);

    QDomNode l = element.firstChild();
    while(!l.isNull()) {
        QDomElement field = l.toElement();

        if(field.tagName() == "data") {
            QString encoding(field.attribute("encoding"));
            QString compression(field.attribute("compression"));

            if(encoding == "base64") {
                QByteArray decoded(QByteArray::fromBase64(qPrintable(field.text())));

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

        l = l.nextSibling();
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

