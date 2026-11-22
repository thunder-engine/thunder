#ifndef LEGACYSPRITEEDIT_H
#define LEGACYSPRITEEDIT_H

#include <editor/legacyasseteditor.h>

#include <resource.h>

class SpriteRender;
class TextureConverter;

class LegacySpriteController;

class SpriteProxy;

class Scene;

namespace Ui {
    class LegacySpriteEdit;
}

class LegacySpriteEdit : public LegacyAssetEditor {
    Q_OBJECT

public:
    LegacySpriteEdit();
    ~LegacySpriteEdit();

    void onUpdateTemplate();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &) override;

    bool allowSaveAs() const override { return false; }

    StringList suffixes() const override;
    TString assetType() const override;

    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    Object::ObjectList selected() const override;

    static void resourceUpdated(int state, void *ptr);

    Ui::LegacySpriteEdit *ui;

    Resource *m_resource;

    SpriteRender *m_render;
    SpriteRender *m_checker;

    TextureConverter *m_converter;

    World *m_world;

    Scene *m_scene;

    LegacySpriteController *m_controller;

    SpriteProxy *m_proxy;

};

class SpriteProxy : public Object {
    A_OBJECT(SpriteProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(SpriteProxy::onUpdated)
    )
public:
    void setEditor(LegacySpriteEdit *editor) {
        m_editor = editor;
    }

    void onUpdated() {
        m_editor->onUpdateTemplate();
    }

private:
    LegacySpriteEdit *m_editor = nullptr;

};

#endif // LEGACYSPRITEEDIT_H
