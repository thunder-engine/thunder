#ifndef ANIMCONVERTER_H
#define ANIMCONVERTER_H

#include "converters/converter.h"
#include "resources/animationclip.h"

class AnimConverter : public IConverter {
public:
    AnimConverter               () {}

    QStringList                 suffixes                    () const { return {"anim"}; }
    uint32_t                    contentType                 () const { return ContentAnimation; }
    uint32_t                    type                        () const { return MetaType::type<AnimationClip *>(); }

    uint8_t                     convertFile                 (IConverterSettings *s);
};

#endif // ANIMCONVERTER_H
