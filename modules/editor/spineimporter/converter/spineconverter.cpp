#include "spineconverter.h"

#include <cstring>

#include <json.h>
#include <bson.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>

#include <resources/mesh.h>
#include <resources/sprite.h>
#include <resources/prefab.h>
#include <resources/material.h>

#include <editor/projectsettings.h>
#include <editor/assetmanager.h>

#define FORMAT_VERSION 1

static std::hash<std::string> hash_str;

enum class SkinTypes {
    Region = 0,
    Mesh,
    LinkedMesh,
    BoundingBox,
    Path,
    Point,
    Clipping
};

std::map<std::string, SkinTypes> gTypeMap = {
    {"region", SkinTypes::Region},
    {"mesh", SkinTypes::Mesh},
    {"linkedmesh", SkinTypes::LinkedMesh},
    {"boundingbox", SkinTypes::BoundingBox},
    {"path", SkinTypes::Path},
    {"point", SkinTypes::Point},
    {"clipping", SkinTypes::Clipping},
};

SpineConverterSettings::SpineConverterSettings() :
        m_scale(1.00f) {
    setType(MetaType::type<Prefab *>());
    setVersion(FORMAT_VERSION);
}

QString SpineConverterSettings::defaultIcon(QString) const {
    return ":/Style/images/spine.svg";
}

float SpineConverterSettings::customScale() const {
    return m_scale;
}
void SpineConverterSettings::setCustomScale(float value) {
    if(m_scale != value) {
        m_scale = value;
        emit updated();
    }
}

AssetConverter::ReturnCode SpineConverter::convertFile(AssetConverterSettings *settings) {
    SpineConverterSettings *spineSettings = static_cast<SpineConverterSettings *>(settings);

    QFile file(settings->source());
    if(file.open(QIODevice::ReadOnly)) {
        std::string data = file.readAll().toStdString();
        file.close();

        Variant spine = Json::load(data);
        VariantMap sections = spine.toMap();

        // Bones section
        auto it = sections.find(gBones);
        if(it != sections.end()) {
            spineSettings->m_root = importBones(it->second.value<VariantList>(), spineSettings);
        }

        // Slots section
        it = sections.find(gSlots);
        if(it != sections.end()) {
            importSlots(it->second.value<VariantList>(), spineSettings);
        }

        // Skins section
        it = sections.find(gSkins);
        if(it != sections.end()) {
            importSkins(it->second.value<VariantList>(), spineSettings);
        }

        // Animations section
        it = sections.find(gAnimations);
        if(it != sections.end()) {
            importAnimations(it->second.value<VariantMap>(), spineSettings);
        }

        /// \todo Constraints section

        /// \todo Events section

        if(spineSettings->m_root) {
            stabilizeUUID(spineSettings->m_root);

            Prefab *prefab = Engine::objectCreate<Prefab>("");
            prefab->setActor(spineSettings->m_root);

            QFile file(settings->absoluteDestination());
            if(file.open(QIODevice::WriteOnly)) {
                ByteArray data = Bson::save(Engine::toVariant(prefab));
                file.write(reinterpret_cast<const char *>(&data[0]), data.size());
                file.close();
            }

            settings->setCurrentVersion(settings->version());

            return Success;
        }
    }

    return InternalError;
}

Actor *SpineConverter::importBones(const VariantList &bones, SpineConverterSettings *settings) {
    Actor *result = nullptr;

    float scale = settings->customScale();

    for(auto &bone : bones) {
        VariantMap fields = bone.value<VariantMap>();

        Actor *parent = nullptr;
        std::string parentField = fields[gParent].toString();
        std::string name = fields[gName].toString();

        if(!parentField.empty()) {
            auto it = settings->m_boneStructure.find(parentField);
            if(it != settings->m_boneStructure.end()) {
                parent = it->second;
            }
        }

        Actor *actor = Engine::objectCreate<Actor>(name, parent);
        actor->addComponent(gTransform);

        settings->m_boneStructure[name] = actor;

        if(parentField.empty()) {
            result = actor;
        }

        Transform *t = actor->transform();

        Vector3 pos;
        auto it = fields.find(gX);
        if(it != fields.end()) {
            pos.x = it->second.toFloat() * scale;
        }
        it = fields.find(gY);
        if(it != fields.end()) {
            pos.y = it->second.toFloat() * scale;
        }
        t->setPosition(pos);

        Vector3 rot;
        it = fields.find(gRotation);
        if(it != fields.end()) {
            rot.z = it->second.toFloat();
        }
        t->setRotation(rot);

        Vector3 scl(1.0f);
        it = fields.find(gScaleX);
        if(it != fields.end()) {
            scl.x = it->second.toFloat();
        }
        it = fields.find(gScaleY);
        if(it != fields.end()) {
            scl.y = it->second.toFloat();
        }
        t->setScale(scl);
    }

    return result;
}

void SpineConverter::importSlots(const VariantList &list, SpineConverterSettings *settings) {
    uint32_t layer = 0;
    for(auto &slot : list) {
        VariantMap slotFields = slot.value<VariantMap>();

        Slot currentSlot;
        currentSlot.layer = layer;

        auto it = slotFields.find(gBone);
        if(it != slotFields.end()) {
            currentSlot.bone = it->second.toString();
        }

        std::string key;

        it = slotFields.find(gName);
        if(it != slotFields.end()) {
            key = it->second.toString();
        }

        it = slotFields.find(gColor);
        if(it != slotFields.end()) {
            currentSlot.color = it->second.toString();
        }

        it = slotFields.find(gAttachment);
        if(it != slotFields.end()) {
            currentSlot.item = it->second.toString();
        }

        /// \todo: blend

        settings->m_slots[key] = currentSlot;

        layer++;
    }
}

void SpineConverter::importSkins(const VariantList &list, SpineConverterSettings *settings) {
    for(auto &skin : list) {
        VariantMap skinFields = skin.value<VariantMap>();

        auto skinIt = skinFields.find(gAttachments);
        if(skinIt != skinFields.end()) {
            Sprite *sprite = Engine::objectCreate<Sprite>(skinFields[gName].toString());

            importAtlas(sprite, settings);

            for(auto &slotIt : skinIt->second.value<VariantMap>()) {
                Slot &slot = settings->m_slots[slotIt.first];

                Actor *bone = settings->m_boneStructure[slot.bone];

                Actor *slotActor = Engine::composeActor(gSpriteRender, slotIt.first, bone);

                for(auto &attachmentIt : slotIt.second.value<VariantMap>()) {
                    std::string attachmentName = attachmentIt.first;

                    VariantMap attachmentFields = attachmentIt.second.value<VariantMap>();

                    SkinTypes type = SkinTypes::Region;

                    auto it = attachmentFields.find(gType);
                    if(it != attachmentFields.end()) {
                        type = gTypeMap[it->second.toString()];
                    }

                    it = attachmentFields.find(gPath);
                    if(it != attachmentFields.end()) {
                        attachmentName = it->second.toString();
                    }

                    Mesh *mesh = Engine::objectCreate<Mesh>(attachmentName);
                    switch(type) {
                        case SkinTypes::Region: {
                            importRegion(attachmentFields, attachmentName, slotActor->transform(), mesh, settings);
                        } break;
                        case SkinTypes::Mesh: {
                            importMesh(attachmentFields, attachmentName, mesh, settings);
                        } break;
                        default: break;
                    }

                    if(!mesh->vertices().empty()) {
                        mesh->setColors(Vector4Vector(mesh->vertices().size(), Vector4(1.0f)));
                        mesh->recalcBounds();

                        QString uuid = settings->saveSubData(Bson::save(ObjectSystem::toVariant(mesh)), mesh->name().c_str(), MetaType::type<Mesh *>());
                        Engine::setResource(mesh, uuid.toStdString());

                        sprite->setShape(hash_str(attachmentName), mesh);
                    } else {
                        delete mesh;
                    }
                }

                slot.render = static_cast<SpriteRender *>(slotActor->component(gSpriteRender));
                if(slot.render) {
                    slot.render->setItem(slot.item);
                    slot.render->setSprite(sprite);
                    slot.render->setLayer(slot.layer);
                    if(!slot.color.empty()) {
                        slot.render->setColor(toColor(slot.color));
                    }
                }
            }

            QString uuid = settings->saveSubData(Bson::save(ObjectSystem::toVariant(sprite)), sprite->name().c_str(), MetaType::type<Sprite *>());
            Engine::setResource(sprite, uuid.toStdString());
        }
    }
}

void SpineConverter::importAtlas(Sprite *sprite, SpineConverterSettings *settings) {
    QFileInfo info(settings->source());
    QFile file(info.absolutePath() + "/" + info.baseName() + ".atlas");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        enum State {
            FileName = 0,
            Size,
            Filter,
            Pma,
            Name,
            Value
        };

        std::string itemName;
        Vector4 bounds;

        uint32_t currentState = State::FileName;
        for(auto &it : data.split('\n')) {
            switch(currentState) {
                case State::FileName: {
                    std::string path = (info.absolutePath() + "/" + it).toStdString();
                    std::string guid = AssetManager::instance()->pathToGuid(path);

                    Texture *texture = static_cast<Texture *>(Engine::loadResource(guid));
                    if(texture) {
                        sprite->addPage(texture);
                    }
                    currentState++;
                } break;
                case State::Size: {
                    auto list = it.split(':');
                    if(list.front() == "size") {
                        auto values = list.back().split(',');
                        settings->m_atlasSize.x = values.front().toFloat();
                        settings->m_atlasSize.y = values.back().toFloat();
                        currentState++;
                    }
                } break;
                case State::Filter:
                case State::Pma: {
                    currentState++;
                } break;
                case State::Name: {
                    itemName = it.toStdString();
                    currentState++;
                } break;
                case State::Value: {
                    auto list = it.split(':');
                    if(list.size() == 1) {
                        itemName = it.toStdString();
                    } else {
                        if(list.front() == gBounds) {
                            auto values = list.back().split(',');

                            for(uint32_t i = 0; i < 4; i++) {
                                settings->m_atlasItems[itemName].bounds[i] = values[i].toFloat();
                            }
                        } else if(list.front() == gOffsets) {

                        } else if(list.front() == gRotate) {
                            settings->m_atlasItems[itemName].rotate = list.back().toFloat();
                        }
                    }
                } break;
                default: break;
            }
        }
    }
}

void SpineConverter::importRegion(const VariantMap &fields, const std::string &itemName, Transform *transform, Mesh *mesh, SpineConverterSettings *settings) {
    float customScale = settings->customScale();

    Item item = settings->m_atlasItems[itemName];

    Vector4 bounds(item.bounds);
    if(item.rotate > 0) {
        float tmp = bounds.z;
        bounds.z = bounds.w;
        bounds.w = tmp;
    }

    float x = bounds.x / settings->m_atlasSize.x;
    bounds.x = x;
    bounds.z = x + bounds.z / settings->m_atlasSize.x;

    float y = 1.0f - bounds.y / settings->m_atlasSize.y;
    bounds.y = y;
    bounds.w = y - bounds.w / settings->m_atlasSize.y;

    Vector3 pos(transform->position());
    auto it = fields.find(gX);
    if(it != fields.end()) {
        pos.x = stof(it->second.toString()) * customScale;
    }
    it = fields.find(gY);
    if(it != fields.end()) {
        pos.y = stof(it->second.toString()) * customScale;
    }
    transform->setPosition(pos);

    Vector3 rot(transform->rotation());
    it = fields.find(gRotation);
    if(it != fields.end()) {
        rot.z = stof(it->second.toString());
    }
    transform->setRotation(rot);

    Vector3 scl(transform->scale());
    it = fields.find(gScaleX);
    if(it != fields.end()) {
        scl.x = stof(it->second.toString());
    }
    it = fields.find(gScaleY);
    if(it != fields.end()) {
        scl.y = stof(it->second.toString());
    }
    transform->setScale(scl);

    Vector2 size;
    it = fields.find(gWidth);
    if(it != fields.end()) {
        size.x = stof(it->second.toString()) * customScale;
    }
    it = fields.find(gHeight);
    if(it != fields.end()) {
        size.y = stof(it->second.toString()) * customScale;
    }

    Vector3Vector vertices = {
        {-size.x * 0.5f,  size.y * 0.5f, 0.0f},
        { size.x * 0.5f,  size.y * 0.5f, 0.0f},
        { size.x * 0.5f, -size.y * 0.5f, 0.0f},
        {-size.x * 0.5f, -size.y * 0.5f, 0.0f}
    };

    Matrix3 m;
    m.rotate(Vector3(0.0f, 0.0f, 1.0f), -item.rotate);

    for(auto &vertex : vertices) {
        vertex = m * vertex;
    }

    Vector2Vector uvs = {
        {bounds.x, bounds.y},
        {bounds.z, bounds.y},
        {bounds.z, bounds.w},
        {bounds.x, bounds.w},
    };

    IndexVector indices = {0, 1, 2, 0, 2, 3};

    mesh->setIndices(indices);
    mesh->setVertices(vertices);
    mesh->setUv0(uvs);
}

void SpineConverter::importMesh(const VariantMap &fields, const std::string &itemName, Mesh *mesh, SpineConverterSettings *settings) {
    float customScale = settings->customScale();

    Item item = settings->m_atlasItems[itemName];

    uint32_t uvCount = 0;
    uint32_t vertexCount = 0;

    Vector4 bounds(item.bounds);
    if(item.rotate > 0) {
        float tmp = bounds.z;
        bounds.z = bounds.w;
        bounds.w = tmp;
    }

    float x = bounds.x / settings->m_atlasSize.x;
    bounds.x = x;
    bounds.z = x + bounds.z / settings->m_atlasSize.x;

    float y = 1.0f - bounds.y / settings->m_atlasSize.y;
    bounds.y = y;
    bounds.w = y - bounds.w / settings->m_atlasSize.y;

    auto it = fields.find("uvs");
    if(it != fields.end()) {
        Vector2Vector uvs;

        VariantList list = it->second.toList();
        uvs.resize(list.size() / 2);
        for(auto &uv : list) {
            float v0 = 0;
            float v1 = 0;

            uint32_t comp = 0;

            uint32_t index = uvCount / 2;
            if(item.rotate > 0) {
                comp = (uvCount + 1) % 2;
                v0 = bounds[comp];
                v1 = bounds[comp + 2];

                if(item.rotate > 0 && comp == 1) {
                    float tmp = v0;
                    v0 = v1;
                    v1 = tmp;
                }
            } else {
                comp = uvCount % 2;
                v0 = bounds[comp];
                v1 = bounds[comp + 2];
            }

            uvs[index][comp] = MIX(v0, v1, uv.toFloat());

            uvCount++;
        }

        uvCount /= 2;

        mesh->setUv0(uvs);
    }

    it = fields.find(gVertices);
    if(it != fields.end()) {
        Vector3Vector &vertices = mesh->vertices();

        vertices.resize(uvCount);

        VariantList list = it->second.toList();
        if(list.size() > uvCount * 2) { // weighted mesh
            Vector4Vector &bones = mesh->bones();
            Vector4Vector &weights = mesh->weights();

            bones.resize(uvCount);
            weights.resize(uvCount);

            auto vertexIt = list.begin();
            while(vertexIt != list.end()) {
                int32_t wCount = vertexIt->toInt();
                ++vertexIt;

                Vector3 vertex;

                for(int32_t w = 0; w < wCount; w++) {
                    int bone = vertexIt->toInt();
                    ++vertexIt;
                    float posX = vertexIt->toFloat();
                    ++vertexIt;
                    float posY = vertexIt->toFloat();
                    ++vertexIt;
                    float weight = vertexIt->toFloat();
                    ++vertexIt;

                    if(w < 4) {
                        bones[vertexCount][w] = bone;
                        weights[vertexCount][w] = weight;

                        vertex += Vector3(posX, posY, 0.0f) * weight;
                    }
                }

                vertices[vertexCount] = vertex;

                vertexCount++;
            }
        } else {
            for(auto &vertex : list) {
                uint32_t index = vertexCount / 2;
                vertices[index][vertexCount % 2] = vertex.toFloat() * customScale;

                vertexCount++;
            }
        }
    }

    it = fields.find(gTriangles);
    if(it != fields.end()) {
        IndexVector &indices = mesh->indices();

        VariantList list = it->second.toList();
        indices.reserve(list.size());
        for(auto &index : list) {
            indices.push_back(index.toInt());
        }
    }
}

Vector4 SpineConverter::toColor(const std::string &color) {
    uint32_t rgba = stoul(color, nullptr, 16);
    uint8_t rgb[4];
    rgb[0] = rgba;
    rgb[1] = rgba >> 8;
    rgb[2] = rgba >> 16;
    rgb[3] = rgba >> 24;

    return Vector4((float)rgb[3] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[0] / 255.0f);
}

std::string SpineConverter::pathTo(Object *root, Object *dst) {
    std::string result;
    if(root != dst) {
        std::string parent = pathTo(root, dst->parent());
        if(!parent.empty()) {
            result += parent + "/";
        }
        result += dst->name();
    }
    return result;
}

void SpineConverter::stabilizeUUID(Object *object) {
    Engine::replaceUUID(object, hash_str(pathTo(nullptr, object)));

    for(auto it : object->getChildren()) {
        stabilizeUUID(it);
    }
}

AssetConverterSettings *SpineConverter::createSettings() const {
    return new SpineConverterSettings();
}

Actor *SpineConverter::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    Resource *resource = Engine::loadResource<Resource>(guid.toStdString());
    Prefab *prefab = dynamic_cast<Prefab *>(resource);
    if(prefab) {
        Actor *actor = prefab->actor();
        return static_cast<Actor *>(actor->clone());
    }
    return AssetConverter::createActor(settings, guid);
}
