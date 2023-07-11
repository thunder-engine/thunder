#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <editor/asseteditor.h>

#include <resources/resource.h>

class SpriteRender;
class TextureConverter;

class SpriteController;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public AssetEditor {
    Q_OBJECT

public:
    TextureEdit();
    ~TextureEdit();

private:
    void loadAsset(AssetConverterSettings *settings) override;

    QStringList suffixes() const override;

    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    static void resourceUpdated(int state, void *ptr);

    Ui::TextureEdit *ui;

    Resource *m_resource;

    SpriteRender *m_render;

    TextureConverter *m_converter;

    World *m_graph;

    Scene *m_scene;

    SpriteController *m_controller;

private slots:
    void onUpdateTemplate();

};

#endif // TEXTUREEDIT_H
