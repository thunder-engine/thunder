#ifndef MESHEDIT_H
#define MESHEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

class Engine;
class Actor;
class DirectLight;

class SceneView;
class NextObject;
class IconRender;

namespace Ui {
    class MeshEdit;
}

class MeshEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    MeshEdit                (Engine *engine);
    ~MeshEdit               ();

    void                    readSettings                                ();
    void                    writeSettings                               ();
    void                    closeEvent                                  (QCloseEvent *event);

    void                    loadAsset                                   (IConverterSettings *settings);

signals:
    void                    templateUpdate                              ();

private:
    void                    prepareScene                                (const QString &resource);

    void                    timerEvent                                  (QTimerEvent *event);

    Ui::MeshEdit           *ui;

    Actor                  *m_pMesh;
    Actor                  *m_pGround;
    Actor                  *m_pDome;
    Actor                  *m_pLight;

    NextObject             *m_pEditor;

    SceneView              *glWidget;

private slots:
    void                    onGLInit                                    ();

    void                    onKeyPress                                  (QKeyEvent *pe);
    void                    onKeyRelease                                (QKeyEvent *pe);

    void                    onToolWindowActionToggled                   (bool checked);

    void                    onToolWindowVisibilityChanged               (QWidget *toolWindow, bool visible);

};

#endif // MESHEDIT_H
