#ifndef SPRITEEDIT_H
#define SPRITEEDIT_H

#include <editor/asseteditor.h>

#include <resources/resource.h>

class SpriteRender;
class TextureConverter;

class SpriteController;

namespace Ui {
    class SpriteEdit;
}

class SpriteEdit : public AssetEditor {
    Q_OBJECT

public:
    SpriteEdit();
    ~SpriteEdit();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &) override;

    bool allowSaveAs() const override { return false; }

    QStringList suffixes() const override;

    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    static void resourceUpdated(int state, void *ptr);

    Ui::SpriteEdit *ui;

    Resource *m_resource;

    SpriteRender *m_render;
    SpriteRender *m_checker;

    TextureConverter *m_converter;

    World *m_world;

    Scene *m_scene;

    SpriteController *m_controller;

private slots:
    void onUpdateTemplate();

};

#endif // SPRITEEDIT_H
