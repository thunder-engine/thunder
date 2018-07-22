#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

class Viewport;
class Engine;

class Texture;
class SpriteMesh;
class TextureConverter;
class TextureImportSettings;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    TextureEdit                 (Engine *engine);
    ~TextureEdit                ();

    void                        timerEvent          (QTimerEvent *);

    void                        readSettings        ();
    void                        writeSettings       ();

    void                        loadAsset           (IConverterSettings *settings);

signals:
    void                        templateUpdate      ();

private:
    void                        closeEvent          (QCloseEvent *event);

    Ui::TextureEdit            *ui;

    Texture                    *m_pTexture;

    SpriteMesh                 *m_pSprite;

    TextureImportSettings      *m_pSettings;

    TextureConverter           *m_pConverter;

private slots:
    void                        onUpdateTemplate                (bool update = true);

    void                        onGLInit                        ();

    void                        onToolWindowActionToggled       (bool checked);

    void                        onToolWindowVisibilityChanged   (QWidget *toolWindow, bool visible);

    void                        on_actionSave_triggered         ();

};

#endif // TEXTUREEDIT_H
