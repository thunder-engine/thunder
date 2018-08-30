#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "core/variant.h"

class NEXT_LIBRARY_EXPORT KeyFrame {
public:
    enum Type {
        Linear                  = 0,
        Cubic
    };

public:
    KeyFrame                    ();

    KeyFrame                    (uint32_t position, Variant &value);

    KeyFrame                    (uint32_t position, Variant &value, Variant &support);

    uint32_t                    mPosition;

    Type                        mType;

    Variant                     mValue;

    Variant                     mSupport;
};

#endif // KEYFRAME_H
