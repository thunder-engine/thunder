#ifndef TEXTUREEDIT_H
#define TEXTUREEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

#include "textureimportsettings.h"

class SceneView;
class Engine;

class Texture;
class Sprite;
class TextureConverter;

namespace Ui {
    class TextureEdit;
}

class TextureEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    TextureEdit                 (Engine *engine, QGLWidget *share);
    ~TextureEdit                ();

    void                        timerEvent          (QTimerEvent *event);

    void                        readSettings        ();
    void                        writeSettings       ();

    void                        loadAsset           (IConverterSettings *settings);

signals:
    void                        templateUpdate      ();

private:
    void                        closeEvent          (QCloseEvent *event);

    Ui::TextureEdit            *ui;

    SceneView                  *glWidget;

    Texture                    *m_pTexture;

    Sprite                     *m_pSprite;

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
