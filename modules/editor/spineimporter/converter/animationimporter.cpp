#include "spineconverter.h"

#include <bson.h>
#include <log.h>

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

void importBoneTransform(TransformMode mode, AnimationClip &clip, const TString &path, const Vector3 &value, const Variant &variant) {
    AnimationTrack track;
    track.setPath(path);

    static const std::vector<TString> properties = {
        "position",
        "rotation",
        "scale"
    };

    track.setProperty(properties[mode]);

    AnimationCurve &curve = track.curve();

    AnimationCurve::KeyFrame frame;
    frame.m_type = AnimationCurve::KeyFrame::Linear;
    frame.m_position = 0.0f;
    frame.m_value = value;

    curve.m_keys.push_back(frame);

    for(auto &key : variant.value<VariantList>()) {
        VariantMap fields = key.value<VariantMap>();

        float time = 0.0f;
        auto it = fields.find(gTime);
        if(it != fields.end()) {
            time = it->second.toFloat();
        }
        frame.m_position = time;

        Vector3 v = value;

        if(mode == TransformMode::Rotate) {
            it = fields.find(gAngle);
            if(it != fields.end()) {
                v.z += it->second.toFloat();
            } else {
                it = fields.find("value");
                if(it != fields.end()) {
                    v.z += it->second.toFloat();
                }
            }
        } else {
            it = fields.find(gX);
            if(it != fields.end()) {
                if(mode == TransformMode::Translate) {
                    v.x = value.x + it->second.toFloat();
                } else {
                    v.x = value.x * it->second.toFloat();
                }
            }

            it = fields.find(gY);
            if(it != fields.end()) {
                if(mode == TransformMode::Translate) {
                    v.y = value.y + it->second.toFloat();
                } else {
                    v.y = value.y * it->second.toFloat();
                }
            }
        }

        frame.m_value = v;

        curve.m_keys.push_back(frame);
    }

    clip.m_tracks.push_back(track);
}

void importBoneTimeline(const VariantMap &bones, AnimationClip &clip, SpineConverterSettings *settings) {
    for(auto &bone : bones) {
        Actor *boneActor = settings->m_boneStructure[bone.first];

        Transform *t = boneActor->transform();

        TString path = SpineConverter::pathTo(settings->m_root, t);

        for(auto &type : bone.second.value<VariantMap>()) {
            if(type.first == gRotate) {
                importBoneTransform(TransformMode::Rotate, clip, path, t->rotation(), type.second);

            } else if(type.first == gTranslate) {
                importBoneTransform(TransformMode::Translate, clip, path, t->position(), type.second);

            } else if(type.first == gScale) {
                importBoneTransform(TransformMode::Scale, clip, path, t->scale(), type.second);

            } else if(type.first == "shear") {
                // Unimplemented
            }
        }
    }
}

void importSlotTimeline(const VariantMap &slotes, AnimationClip &clip, SpineConverterSettings *settings) {
    for(auto &slotIt : slotes) {
        Slot &slot = settings->m_slots[slotIt.first];

        TString path = SpineConverter::pathTo(settings->m_root, slot.render);

        for(auto &type : slotIt.second.value<VariantMap>()) {
            if(type.first == gAttachment) {
                AnimationTrack track;

                track.setPath(path);
                track.setProperty("item");

                AnimationCurve &curve = track.curve();

                AnimationCurve::KeyFrame frame;
                frame.m_type = AnimationCurve::KeyFrame::Constant;
                frame.m_position = 0.0f;
                frame.m_value = slot.render->item();

                curve.m_keys.push_back(frame);

                for(auto &key : type.second.value<VariantList>()) {
                    VariantMap fields = key.value<VariantMap>();

                    frame.m_position = 0.0f;
                    auto it = fields.find(gTime);
                    if(it != fields.end()) {
                        frame.m_position = it->second.toFloat();
                    }

                    TString value = slot.render->item();

                    it = fields.find(gName);
                    if(it != fields.end()) {
                        value = it->second.toString();
                    }

                    frame.m_value = value;

                    curve.m_keys.push_back(frame);
                }

                clip.m_tracks.push_back(track);
            } else if(type.first == gColor || type.first == "twoColor" || type.first == "rgba" || type.first == "rgba2") {
                AnimationTrack track;

                track.setPath(path);
                track.setProperty(gColor);

                AnimationCurve &curve = track.curve();

                AnimationCurve::KeyFrame frame;
                frame.m_type = AnimationCurve::KeyFrame::Linear;
                frame.m_position = 0.0f;
                frame.m_value = slot.render->color();

                curve.m_keys.push_back(frame);

                for(auto &key : type.second.value<VariantList>()) {
                    VariantMap fields = key.value<VariantMap>();

                    frame.m_position = 0.0f;
                    auto it = fields.find(gTime);
                    if(it != fields.end()) {
                        frame.m_position = it->second.toFloat();
                    }

                    Vector4 value = slot.render->color();

                    it = fields.find(gColor);
                    if(it != fields.end()) {
                        value = SpineConverter::toColor(it->second.toString());
                    }

                    frame.m_value = value;

                    curve.m_keys.push_back(frame);
                }

                clip.m_tracks.push_back(track);
            }
        }
    }
}

void importDrawOrderTimeline(const VariantList &keys, AnimationClip &clip, SpineConverterSettings *settings) {
    std::map<TString, AnimationTrack> tracks;

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
                track.setPath(SpineConverter::pathTo(settings->m_root, slot.render));

                AnimationCurve::KeyFrame frame;
                frame.m_type = AnimationCurve::KeyFrame::Constant;
                frame.m_value = slot.render->layer();
                frame.m_position = 0.0f;

                AnimationCurve &curve = track.curve();
                curve.m_keys.push_back(frame);

                tracks[slotName] = track;
            }

            int value = slot.render->layer();
            auto it = offsetFields.find(gOffset);
            if(it != offsetFields.end()) {
                value += offsetFields[gOffset].toInt();
            }

            AnimationCurve::KeyFrame frame;
            frame.m_type = AnimationCurve::KeyFrame::Constant;
            frame.m_value = value;
            frame.m_position = time;

            AnimationCurve &curve = tracks[slotName].curve();
            curve.m_keys.push_back(frame);
        }
    }

    for(auto &it : tracks) {
        clip.m_tracks.push_back(it.second);
    }
}

void SpineConverter::importAnimations(const VariantMap &animations, SpineConverterSettings *settings) {
    for(auto &animation : animations) {
        AnimationClip *clip = Engine::objectCreate<AnimationClip>(animation.first);

        for(auto &timeline : animation.second.value<VariantMap>()) {
            if(timeline.first == gBones) {
                importBoneTimeline(timeline.second.value<VariantMap>(), *clip, settings);
            } else if(timeline.first == gSlots) {
                importSlotTimeline(timeline.second.value<VariantMap>(), *clip, settings);
            } else if(QString(timeline.first.data()).toLower() == gDrawOrder) {
                importDrawOrderTimeline(timeline.second.value<VariantList>(), *clip, settings);
            }
        }

        for(auto &it : clip->m_tracks) {
            float duration = 0.0f;
            AnimationCurve &curve = it.curve();
            if(!curve.m_keys.empty()) {
                duration = MAX(curve.m_keys.back().m_position, duration);
            }

            it.setDuration(duration * 1000.0f);

            // Normalize
            for(auto &key : curve.m_keys) {
                key.m_position /= duration;
            }
        }

        QString uuid = settings->saveSubData(Bson::save(ObjectSystem::toVariant(clip)), clip->name().data(), MetaType::type<AnimationClip *>());
        Engine::setResource(clip, uuid.toStdString());
    }
}
