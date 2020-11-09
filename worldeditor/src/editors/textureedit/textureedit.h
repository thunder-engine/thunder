#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <QWidget>

#include "editors/scenecomposer/documentmodel.h"

class Viewport;
class Engine;

class Sprite;
class Texture;
class SpriteRender;
class TextureConverter;
class TextureImportSettings;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public QWidget, public IAssetEditor {
    Q_OBJECT

public:
    TextureEdit(DocumentModel *document);
    ~TextureEdit();

signals:
    void templateUpdate();

private:
    void loadAsset(IConverterSettings *settings) override;

    QStringList assetTypes() const override;

    void timerEvent(QTimerEvent *) override;
    void closeEvent(QCloseEvent *event) override;
    bool isModified() const override;

    Ui::TextureEdit *ui;

    SpriteRender *m_pRender;

    IConverterSettings *m_pSettings;

    TextureConverter *m_pConverter;

    DocumentModel *m_pDocument;

    QString m_Path;

private slots:
    void onUpdateTemplate();

    void onGLInit();

};

#endif // TEXTUREEDIT_H
