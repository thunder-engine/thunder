#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <editor/asseteditor.h>

#include <resources/resource.h>

class Sprite;
class SpriteRender;
class TextureConverter;
class TextureImportSettings;

class SpriteElement;

class DocumentModel;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public AssetEditor, public Resource::IObserver {
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

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override;

    Ui::TextureEdit *ui;

    Resource *m_Rresource;

    SpriteRender *m_pRender;

    TextureConverter *m_pConverter;

    SpriteElement *m_Details;

private slots:
    void onUpdateTemplate();

};

#endif // TEXTUREEDIT_H
