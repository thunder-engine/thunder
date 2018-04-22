#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

#include <components/staticmesh.h>

using namespace std;

class Engine;
class DirectLight;

class Viewport;
class ShaderBuilder;

namespace Ui {
    class MaterialEdit;
}

class MaterialEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT
    
public:
    MaterialEdit            (Engine *engine);
    ~MaterialEdit           ();

    void                    readSettings                    ();
    void                    writeSettings                   ();

    void                    loadAsset                       (IConverterSettings *settings);

signals:
    void                    templateUpdate                  ();

private:
    void                    changeMesh                      (const string &path);
    void                    closeEvent                      (QCloseEvent *event);
    void                    timerEvent                      (QTimerEvent *);

    Ui::MaterialEdit       *ui;

    Actor                  *m_pMesh;
    Actor                  *m_pLight;

    Material               *m_pMaterial;

    ShaderBuilder          *m_pBuilder;

    QObject                *m_pEditor;

    QString                 m_Path;

    QString                 m_CustomMesh;

    Viewport               *glWidget;

private slots:
    void                    onGLInit                        ();

    void                    onKeyPress                      (QKeyEvent *pe);
    void                    onKeyRelease                    (QKeyEvent *pe);

    void                    onNodeSelected                  (void *node);
    void                    onUpdateTemplate                (bool update = true);

    void                    on_actionPlane_triggered        ();
    void                    on_actionCube_triggered         ();
    void                    on_actionSphere_triggered       ();

    void                    on_actionSave_triggered         ();

    void                    onToolWindowActionToggled       (bool checked);

    void                    onToolWindowVisibilityChanged   (QWidget *toolWindow, bool visible);
};

#endif // MATERIALEDIT_H
