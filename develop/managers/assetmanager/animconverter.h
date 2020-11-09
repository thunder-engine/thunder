#ifndef ANIMCONVERTER_H
#define ANIMCONVERTER_H

#include <editor/converter.h>
#include <resources/animationclip.h>

class AnimImportSettings : public IConverterSettings {
public:
    AnimImportSettings();
};

class AnimConverter : public IConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"anim"}; }

    uint8_t convertFile(IConverterSettings *s) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;

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
