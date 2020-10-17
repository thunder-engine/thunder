#ifndef ANIMCONVERTER_H
#define ANIMCONVERTER_H

#include "converters/converter.h"
#include "resources/animationclip.h"

class AnimConverter : public IConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"anim"}; }
    uint32_t contentType() const Q_DECL_OVERRIDE { return ContentAnimation; }
    uint32_t type() const Q_DECL_OVERRIDE { return MetaType::type<AnimationClip *>(); }

    uint8_t convertFile(IConverterSettings *s) Q_DECL_OVERRIDE;
    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Animation.anim"; }

private:
    Variant readJson(const string &data, IConverterSettings *settings);
    void toVersion1(Variant &variant);
};

class AnimationClipSerial : public AnimationClip {
public:
    VariantMap saveUserData() const;

protected:
    friend class AnimConverter;

};

#endif // ANIMCONVERTER_H
