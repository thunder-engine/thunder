#ifndef MESHEDIT_H
#define MESHEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

class Engine;
class Actor;
class DirectLight;

class Viewport;
class NextObject;

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
    void                    timerEvent                                  (QTimerEvent *);

    Ui::MeshEdit           *ui;

    Actor                  *m_pMesh;
    Actor                  *m_pGround;
    Actor                  *m_pDome;
    Actor                  *m_pLight;

    IConverterSettings     *m_pSettings;

    Viewport               *glWidget;

private slots:
    void                    onGLInit                                    ();

    void                    onUpdateTemplate                            ();

    void                    onToolWindowActionToggled                   (bool checked);

    void                    onToolWindowVisibilityChanged               (QWidget *toolWindow, bool visible);

    void                    on_actionSave_triggered                     ();

};

#endif // MESHEDIT_H
