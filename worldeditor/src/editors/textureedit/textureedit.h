#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

class Viewport;
class Engine;

class Texture;
class SpriteRender;
class TextureConverter;
class TextureImportSettings;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    TextureEdit ();
    ~TextureEdit ();

    void readSettings ();
    void writeSettings ();

    void loadAsset (IConverterSettings *settings) override;

signals:
    void templateUpdate ();

private:
    void timerEvent (QTimerEvent *) override;
    void closeEvent (QCloseEvent *event) override;
    bool isModified() const override;

    bool m_Modified;

    Ui::TextureEdit *ui;

    Texture *m_pTexture;

    SpriteRender *m_pSprite;

    IConverterSettings *m_pSettings;

    TextureConverter *m_pConverter;

private slots:
    void onUpdateTemplate (bool update = true);

    void onGLInit ();

    void onToolWindowActionToggled (bool checked);

    void onToolWindowVisibilityChanged (QWidget *toolWindow, bool visible);

    void on_actionSave_triggered ();

};

#endif // TEXTUREEDIT_H
