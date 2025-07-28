#ifndef SPRITEEDIT_H
#define SPRITEEDIT_H

#include <editor/asseteditor.h>

#include <resources/resource.h>

class SpriteRender;
class TextureConverter;

class SpriteController;

class SpriteProxy;

namespace Ui {
    class SpriteEdit;
}

class SpriteEdit : public AssetEditor {
    Q_OBJECT

public:
    SpriteEdit();
    ~SpriteEdit();

    void onUpdateTemplate();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &) override;

    bool allowSaveAs() const override { return false; }

    StringList suffixes() const override;

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

    SpriteProxy *m_proxy;

};

class SpriteProxy : public Object {
    A_OBJECT(SpriteProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(SpriteProxy::onUpdated)
    )
public:
    void setEditor(SpriteEdit *editor) {
        m_editor = editor;
    }

    void onUpdated() {
        m_editor->onUpdateTemplate();
    }

private:
    SpriteEdit *m_editor = nullptr;

};

#endif // SPRITEEDIT_H
