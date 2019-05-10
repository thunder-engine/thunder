#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

#include <components/meshrender.h>

using namespace std;

class Engine;
class DirectLight;

class Viewport;
class ShaderBuilder;

class ComponentBrowser;

class QWidgetAction;

namespace Ui {
    class MaterialEdit;
}

class MaterialEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT
    
public:
    MaterialEdit(Engine *engine);
    ~MaterialEdit();

    void readSettings();
    void writeSettings();

    void loadAsset(IConverterSettings *settings);

signals:
    void templateUpdate();

private:
    void changeMesh(const string &path);
    void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *);

    Ui::MaterialEdit    *ui;

    Actor               *m_pMesh;
    Actor               *m_pLight;

    Material            *m_pMaterial;

    ShaderBuilder       *m_pBuilder;

    QObject             *m_pEditor;

    QString              m_Path;

    QString              m_CustomMesh;

    Viewport            *glWidget;

    QMenu               *m_pCreateMenu;
    QWidgetAction       *m_pAction;

    ComponentBrowser    *m_pBrowser;

private slots:
    void onGLInit();

    void onComponentSelected(const QString &path);

    void onKeyPress(QKeyEvent *pe);
    void onKeyRelease(QKeyEvent *pe);

    void onNodeSelected(int);
    void onUpdateTemplate(bool update = true);

    void onToolWindowActionToggled(bool checked);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

    void on_actionSave_triggered();

    void on_schemeWidget_customContextMenuRequested(const QPoint &);
};

#endif // MATERIALEDIT_H
