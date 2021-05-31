#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <QWidget>

#include "editors/scenecomposer/documentmodel.h"

#include "resources/resource.h"

class Viewport;
class Engine;

class Sprite;
class Texture;
class SpriteRender;
class TextureConverter;
class TextureImportSettings;

class SpriteElement;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public QWidget, public IAssetEditor, public Resource::IObserver {
    Q_OBJECT

public:
    TextureEdit(DocumentModel *document);
    ~TextureEdit();

signals:
    void templateUpdate();

private:
    void loadAsset(IConverterSettings *settings) override;

    QStringList assetTypes() const override;

    void timerEvent(QTimerEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override;

    Ui::TextureEdit *ui;

    Resource *m_Rresource;

    SpriteRender *m_pRender;

    TextureImportSettings *m_pSettings;

    TextureConverter *m_pConverter;

    DocumentModel *m_pDocument;

    SpriteElement *m_Details;

    QString m_Path;

private slots:
    void onUpdateTemplate();

};

#endif // TEXTUREEDIT_H
