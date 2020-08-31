#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

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
    MaterialEdit();
    ~MaterialEdit();

    void readSettings();
    void writeSettings();

    void loadAsset(IConverterSettings *settings) override;

signals:
    void templateUpdate();

private:
    void changeMesh(const string &path);
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *) override;

    bool isModified() const override;

    bool m_Modified;

    Ui::MaterialEdit *ui;

    Actor *m_pMesh;
    Actor *m_pLight;

    Material *m_pMaterial;

    ShaderBuilder *m_pBuilder;

    QObject *m_pEditor;

    QString m_Path;

    QString m_CustomMesh;

    Viewport *glWidget;

    QMenu *m_pCreateMenu;
    QWidgetAction *m_pAction;

    ComponentBrowser *m_pBrowser;

    QAction *m_pUndo;
    QAction *m_pRedo;

private slots:
    void onGLInit();

    void onComponentSelected(const QString &path);

    void onKeyPress(QKeyEvent *pe);
    void onKeyRelease(QKeyEvent *pe);

    void onNodesSelected(const QVariant &);
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
