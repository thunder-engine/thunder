#include "components/animationcontroller.h"

#include "components/actor.h"
#include "resources/animationclip.h"

#include "timer.h"
#include "anim/propertyanimation.h"

#define CLIP "Clip"

AnimationController::AnimationController() :
        m_pClip(nullptr),
        m_Time(0) {

}

void AnimationController::start() {
    PROFILE_FUNCTION()

    m_Time  = 0;

    for(auto it : m_Properties) {
        it->start();
    }
}

void AnimationController::update() {
    PROFILE_FUNCTION()

    if(!m_Properties.empty() && (*m_Properties.begin())->state() == Animation::RUNNING) {
        setPosition(m_Time + Timer::deltaTime() * 1000);
    }
}

AnimationClip *AnimationController::clip() const {
    PROFILE_FUNCTION()

    return m_pClip;
}

void AnimationController::setClip(AnimationClip *clip) {
    PROFILE_FUNCTION()

    m_pClip = clip;

    for(auto it : m_Properties) {
        delete it;
    }
    m_Properties.clear();

    if(m_pClip) {
        for(auto &it : clip->m_Tracks) {
            Object *target  = findTarget(actor(), it.path);
            PropertyAnimation *property = new PropertyAnimation();
            property->setTarget(target, it.property.c_str());
            property->setKeyFrames(it.curve);

            m_Properties.push_back(property);
        }
    }
}

uint32_t AnimationController::position() const {
    PROFILE_FUNCTION()

    return m_Time;
}

void AnimationController::setPosition(uint32_t ms) {
    PROFILE_FUNCTION()

    m_Time  = ms;
    for(auto it : m_Properties) {
        it->setCurrentTime(m_Time);
    }
}

uint32_t AnimationController::duration() const {
    PROFILE_FUNCTION()

    uint32_t result = 0;
    for(auto it : m_Properties) {
        result  = MAX(result, it->loopDuration());
    }
    return result;
}

void AnimationController::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION()

    Component::loadUserData(data);
    {
        auto it = data.find(CLIP);
        if(it != data.end()) {
            setClip(Engine::loadResource<AnimationClip>((*it).second.toString()));
        }
    }
}

VariantMap AnimationController::saveUserData() const {
    PROFILE_FUNCTION()

    VariantMap result   = Component::saveUserData();
    {
        string ref  = Engine::reference(m_pClip);
        if(!ref.empty()) {
            result[CLIP]    = ref;
        }
    }
    return result;
}

Object *AnimationController::findTarget(Object *src, const string &path) {
    PROFILE_FUNCTION()

    if(src) {
        if(path.empty() || path == src->name()) {
            return src;
        } else {
            string sub  = path;
            int index   = path.find('/', 1);
            if(index > -1) {
                sub     = path.substr(index + 1);
            }
            for(const auto &it : src->getChildren()) {
                Object *o  = findTarget(it, sub);
                if(o) {
                    return o;
                }
            }
        }
    }

    return nullptr;
}
