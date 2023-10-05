#ifndef ANIMATIONCONTROLLERBUILDER_H
#define ANIMATIONCONTROLLERBUILDER_H

#include <editor/assetconverter.h>

#include "animationcontrollergraph.h"

class AnimationBuilderSettings : public AssetConverterSettings {
public:
    AnimationBuilderSettings();
private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class AnimationControllerBuilder : public AssetConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"actl"}; }

    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Animation_Controller.actl"; }

private:
    AnimationControllerGraph m_model;
};

#endif // ANIMATIONCONTROLLERBUILDER_H
