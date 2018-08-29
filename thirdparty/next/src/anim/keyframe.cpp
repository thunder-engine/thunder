#include "anim/keyframe.h"

KeyFrame::KeyFrame() :
        mPosition(0.0f),
        mType(Linear) {

}

KeyFrame::KeyFrame(uint32_t position, Variant &value) :
        mPosition(position),
        mType(Linear),
        mValue(value) {

}

KeyFrame::KeyFrame(uint32_t position, Variant &value, Variant &support) :
        mPosition(position),
        mType(Linear),
        mValue(value),
        mSupport(support) {

}
