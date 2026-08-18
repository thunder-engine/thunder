#include "spineconverter.h"

#include <bson.h>
#include <log.h>
#include <url.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>

#include <resources/sprite.h>
#include <resources/material.h>
#include <resources/animationclip.h>

const char *gDrawOrder("draworder");

enum TransformMode {
    Translate,
    Rotate,
    Scale
};

void appendKeyFrame(AnimationCurve &curve, float position, const Vector3 &value) {
    AnimationCurve::KeyFrame frame;
    frame.m_type = AnimationCurve::KeyFrame::Linear;
    frame.m_position = position;
    frame.m_value = { value.x, value.y, value.z };
    curve.m_keys.push_back(frame);
}

void appendKeyFrame(AnimationCurve &curve, float position, const Vector4 &value) {
    AnimationCurve::KeyFrame frame;
    frame.m_type = AnimationCurve::KeyFrame::Linear;
    frame.m_position = position;
    frame.m_value = { value.x, value.y, value.z, value.w };
    curve.m_keys.push_back(frame);
}

bool needSetupPoseKey(const VariantList &keys) {
    if(keys.empty()) {
        return true;
    }

    const VariantMap firstKey = keys.front().value<VariantMap>();
    auto it = firstKey.find(gTime);
    return it == firstKey.end() || it->second.toFloat() > 0.0f;
}

void importBoneTransform(TransformMode mode, AnimationClip &clip, const TString &path, const Vector3 &value, const Variant &data, SpineConverterSettings *settings) {
    AnimationTrack track;
    track.setPath(path);

    static const std::vector<TString> properties = {
        "position",
        "rotation",
        "scale"
    };

    track.setProperty(properties[mode]);

    AnimationCurve &curve = track.curve();

    const VariantList keys = data.value<VariantList>();
    if(needSetupPoseKey(keys)) {
        appendKeyFrame(curve, 0.0f, value);
    }

    float customScale = settings->customScale();

    for(auto &key : keys) {
        VariantMap fields = key.value<VariantMap>();

        float position = 0.0f;
        auto it = fields.find(gTime);
        if(it != fields.end()) {
            position = it->second.toFloat();
        }

        Vector3 v = value;

        if(mode == TransformMode::Rotate) {
            it = fields.find(gAngle);
            if(it != fields.end()) {
                v.z += it->second.toFloat();
            } else {
                it = fields.find(gValue);
                if(it != fields.end()) {
                    v.z += it->second.toFloat();
                }
            }
        } else {
            it = fields.find(gX);
            if(it != fields.end()) {
                if(mode == TransformMode::Translate) {
                    v.x = value.x + it->second.toFloat() * customScale;
                } else {
                    v.x = value.x * it->second.toFloat();
                }
            }

            it = fields.find(gY);
            if(it != fields.end()) {
                if(mode == TransformMode::Translate) {
                    v.y = value.y + it->second.toFloat() * customScale;
                } else {
                    v.y = value.y * it->second.toFloat();
                }
            }
        }

        appendKeyFrame(curve, position, v);
    }

    clip.addAnimationTrack(track);
}

void importBoneTimeline(const VariantMap &bones, AnimationClip &clip, SpineConverterSettings *settings) {
    for(auto &bone : bones) {
        Transform *t = nullptr;
        for(auto it : settings->m_bones) {
            if(it->name() == bone.first) {
                t = it->transform();
                break;
            }
        }

        TString path = SpineConverter::pathTo(settings->m_bones.front(), t);

        for(auto &type : bone.second.value<VariantMap>()) {
            if(type.first == gRotate) {
                importBoneTransform(TransformMode::Rotate, clip, path, t->rotation(), type.second, settings);
            } else if(type.first == gTranslate) {
                importBoneTransform(TransformMode::Translate, clip, path, t->position(), type.second, settings);
            } else if(type.first == gScale) {
                importBoneTransform(TransformMode::Scale, clip, path, t->scale(), type.second, settings);
            } else if(type.first == "shear") {
                // Unimplemented
            }
        }
    }
}

void importSlotTimeline(const VariantMap &slotes, AnimationClip &clip, SpineConverterSettings *settings) {
    for(auto &slotIt : slotes) {
        Slot &slot = settings->m_slots[slotIt.first];

        TString path = SpineConverter::pathTo(settings->m_bones.front(), slot.render);

        for(auto &type : slotIt.second.value<VariantMap>()) {
            if(type.first == gAttachment) {
                AnimationTrack track;

                track.setPath(path);
                track.setProperty("sprite");

                AnimationTrack::Frames &frames = track.frames();

                const VariantList keys = type.second.value<VariantList>();
                float maxPosition = 0.0f;
                for(auto &key : keys) {
                    VariantMap fields = key.value<VariantMap>();

                    auto it = fields.find(gTime);
                    if(it != fields.end()) {
                        maxPosition = MAX(maxPosition, it->second.toFloat());
                    }
                }

                if(needSetupPoseKey(keys)) {
                    if(slot.item.isEmpty()) {
                        frames.push_back({ TString(), 0.0f });
                    } else {
                        ResourceSystem::ResourceInfo resSprite = settings->subItem(slot.item);
                        frames.push_back({ resSprite.uuid, 0.0f });
                    }
                }

                for(auto &key : keys) {
                    VariantMap fields = key.value<VariantMap>();

                    float position = 0.0f;
                    auto it = fields.find(gTime);
                    if(it != fields.end()) {
                        position = it->second.toFloat();
                    }

                    TString attachmentName;
                    it = fields.find(gName);
                    if(it != fields.end()) {
                        attachmentName = it->second.toString();
                    }

                    if(attachmentName.isEmpty()) {
                        attachmentName = slot.item;
                    }

                    if(attachmentName.isEmpty()) {
                        frames.push_back({ TString(), position });
                    } else {
                        ResourceSystem::ResourceInfo resSprite = settings->subItem(attachmentName);
                        frames.push_back({ resSprite.uuid, position });
                    }
                }

                if(maxPosition > 0.0f) {
                    for(auto &frame : frames) {
                        frame.m_position /= maxPosition;
                    }
                }

                clip.addAnimationTrack(track);
            } else if(type.first == gColor || type.first == "twoColor" || type.first == "rgba" || type.first == "rgba2") {
                AnimationTrack track;

                track.setPath(path);
                track.setProperty(gColor);

                AnimationCurve &curve = track.curve();

                const VariantList keys = type.second.value<VariantList>();
                if(needSetupPoseKey(keys)) {
                    appendKeyFrame(curve, 0.0f, slot.render->color());
                }

                for(auto &key : keys) {
                    VariantMap fields = key.value<VariantMap>();

                    float position = 0.0f;
                    auto it = fields.find(gTime);
                    if(it != fields.end()) {
                        position = it->second.toFloat();
                    }

                    Vector4 value = slot.render->color();

                    it = fields.find(gColor);
                    if(it != fields.end()) {
                        value = SpineConverter::toColor(it->second.toString());
                    }

                    appendKeyFrame(curve, position, value);
                }

                clip.addAnimationTrack(track);
            }
        }
    }
}

void importDrawOrderTimeline(const VariantList &keys, AnimationClip &clip, SpineConverterSettings *settings) {
    std::map<TString, AnimationTrack> tracks;
    bool needInitialState = false;
    if(!keys.empty()) {
        VariantMap firstKey = keys.front().value<VariantMap>();
        auto firstTime = firstKey.find(gTime);
        needInitialState = firstTime == firstKey.end() || firstTime->second.toFloat() > 0.0f;
    }

    for(auto &key : keys) {
        VariantMap keyFields = key.value<VariantMap>();

        float time = keyFields[gTime].toFloat();
        for(auto &offset : keyFields[gOffsets].value<VariantList>()) {
            VariantMap offsetFields = offset.value<VariantMap>();

            TString slotName = offsetFields[gSlot].toString();
            Slot &slot = settings->m_slots[slotName];

            auto trackIt = tracks.find(slotName);
            if(trackIt == tracks.end()) {
                AnimationTrack track;
                track.setProperty("layer");
                track.setPath(SpineConverter::pathTo(settings->m_bones.front(), slot.render));

                if(needInitialState) {
                    AnimationCurve::KeyFrame frame;
                    frame.m_type = AnimationCurve::KeyFrame::Constant;
                    frame.m_value = { static_cast<float>(slot.render->layer()) };
                    frame.m_position = 0.0f;

                    AnimationCurve &curve = track.curve();
                    curve.m_keys.push_back(frame);
                }

                tracks[slotName] = track;
            }

            int value = slot.render->layer();
            auto it = offsetFields.find(gOffset);
            if(it != offsetFields.end()) {
                value += offsetFields[gOffset].toInt();
            }

            AnimationCurve::KeyFrame frame;
            frame.m_type = AnimationCurve::KeyFrame::Constant;
            frame.m_value = { static_cast<float>(value) };
            frame.m_position = time;

            AnimationCurve &curve = tracks[slotName].curve();
            curve.m_keys.push_back(frame);
        }
    }

    for(auto &it : tracks) {
        clip.addAnimationTrack(it.second);
    }
}

void importAttachmentsTimeline(const VariantMap &attachments, AnimationClip &clip, const TString &clipName, SpineConverterSettings *settings) {
    for(auto &attachmentIt : attachments) {
        for(auto &slotIt : attachmentIt.second.value<VariantMap>()) {
            TString slotName = slotIt.first;
            Slot &slot = settings->m_slots[slotName];

            for(auto &itemIt : slotIt.second.value<VariantMap>()) {
                TString spriteName = itemIt.first;
                Item &item = settings->m_atlasItems[spriteName];

                for(auto &type : itemIt.second.value<VariantMap>()) {
                    if(type.first == "deform") {
                        AnimationTrack track;
                        track.setProperty(TString("blendShape:") + clipName);
                        track.setPath(SpineConverter::pathTo(settings->m_bones.front(), slot.render));

                        AnimationCurve &curve = track.curve();

                        // Process blend shape deformations from animation
                        const VariantList deformKeys = type.second.value<VariantList>();
                        if(!deformKeys.empty()) {
                            for(auto &key : deformKeys) {
                                VariantMap fields = key.value<VariantMap>();

                                float position = 0.0f;
                                auto it = fields.find(gTime);
                                if(it != fields.end()) {
                                    position = it->second.toFloat();
                                }

                                // Extract vertex deltas
                                std::vector<uint32_t> indices;
                                Vector3Vector frameVertices;

                                uint32_t offset = 0;
                                auto offsetIt = fields.find("offset");
                                if(offsetIt != fields.end()) {
                                    offset = offsetIt->second.toInt();
                                }

                                auto verticesIt = fields.find("vertices");
                                if(verticesIt != fields.end()) {
                                    VariantList vertices = verticesIt->second.toList();
                                    auto vertexIt = vertices.begin();
                                    uint32_t index = 0;
                                    while(vertexIt != vertices.end()) {
                                        Vector3 delta;
                                        delta.x = vertexIt->toFloat();
                                        vertexIt++;
                                        if(vertexIt != vertices.end()) {
                                            delta.y = vertexIt->toFloat();
                                            vertexIt++;
                                        }

                                        delta *= settings->customScale();

                                        if(delta.x != 0.0f || delta.y != 0.0f) {
                                            indices.push_back(offset + index);
                                            frameVertices.push_back(delta);
                                        }
                                        index++;
                                    }

                                    // Add blend shape frame to mesh if we have data
                                    if(!indices.empty()) {
                                        Sprite *sprite = item.sprite;
                                        if(sprite) {
                                            Mesh *mesh = sprite->mesh();
                                            if(mesh) {
                                                AnimationCurve::KeyFrame frame;
                                                frame.m_type = AnimationCurve::KeyFrame::Linear;
                                                frame.m_value = { static_cast<float>(position) };
                                                frame.m_position = position;
                                                curve.m_keys.push_back(frame);

                                                mesh->addBlendShapeFrame(clipName, position, indices, frameVertices);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if(!curve.m_keys.empty()) {
                            float lastPosition = curve.m_keys.back().m_position;

                            for(auto &it : curve.m_keys) {
                                it.m_position /= lastPosition;
                                it.m_value[0] /= lastPosition;
                            }

                            track.setDuration(lastPosition * 1000.0f);

                            clip.addAnimationTrack(track);
                        }
                    }

                    Sprite *sprite = item.sprite;
                    if(sprite) {
                        Mesh *mesh = sprite->mesh();
                        if(mesh) {
                            for(auto &shape : mesh->blendShapes()) {
                                if(shape.name == clipName && !shape.frames.empty()) {
                                    float lastWeight = shape.frames.back().weight;
                                    if(lastWeight != 0.0f) {
                                        for(auto &it : shape.frames) {
                                            it.weight /= lastWeight;
                                        }
                                    }
                                }
                            }

                            TString spriteUuid(Engine::reference(sprite));

                            Url dst(settings->absoluteDestination());
                            AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(sprite), dst.absoluteDir() + "/" + spriteUuid);
                            if(result != AssetConverter::Success) {
                                aError() << "Unable to update sprite";
                            }
                        }
                    }
                }
            }
        }
    }
}

void SpineConverter::importAnimations(const VariantMap &animations, SpineConverterSettings *settings) {
    for(auto &animation : animations) {
        ResourceSystem::ResourceInfo info = settings->subItem(animation.first, MetaType::name<AnimationClip>());
        AnimationClip *clip = Engine::loadResource<AnimationClip>(info.uuid);
        if(clip == nullptr) {
            clip = Engine::objectCreate<AnimationClip>(info.uuid);
        }

        clip->tracks().clear();

        for(auto &timeline : animation.second.value<VariantMap>()) {
            if(timeline.first == gBones) {
                importBoneTimeline(timeline.second.value<VariantMap>(), *clip, settings);
            } else if(timeline.first == gSlots) {
                importSlotTimeline(timeline.second.value<VariantMap>(), *clip, settings);
            } else if(timeline.first.toLower() == gDrawOrder) {
                importDrawOrderTimeline(timeline.second.value<VariantList>(), *clip, settings);
            } else if(timeline.first == gAttachments) {
                importAttachmentsTimeline(timeline.second.value<VariantMap>(), *clip, animation.first, settings);
            }
        }

        for(auto &it : clip->tracks()) {
            float duration = 0.0f;
            AnimationCurve &curve = it.curve();
            if(!curve.m_keys.empty()) {
                duration = MAX(curve.m_keys.back().m_position, duration);
            }

            for(auto &frame : it.frames()) {
                duration = MAX(frame.m_position, duration);
            }

            it.setDuration(duration * 1000.0f);

            if(duration > 0.0f) { // Normalize
                for(auto &key : curve.m_keys) {
                    key.m_position /= duration;
                }
            }
        }

        Url dst(settings->absoluteDestination());

        AssetConverter::ReturnCode result = settings->saveBinary(Engine::toVariant(clip), dst.absoluteDir() + "/" + info.uuid);
        if(result == AssetConverter::Success) {
            info.id = clip->uuid();
            settings->setSubItem(animation.first, info, 0);
        }
    }
}
