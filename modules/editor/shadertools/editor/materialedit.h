#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <editor/asseteditor.h>

#include <components/meshrender.h>

class ShaderBuilder;
class ShaderNodeGraph;
class CameraCtrl;

class ComponentBrowser;
class UndoCommand;

class QMenu;

namespace Ui {
    class MaterialEdit;
}

class MaterialEdit : public AssetEditor {
    Q_OBJECT
    
public:
    MaterialEdit();
    ~MaterialEdit();

private slots:
    void onActivated() override;

    void onSchemeUpdated();

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    void changeMesh(const string &path);
    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    QStringList suffixes() const override;

private:
    Ui::MaterialEdit *ui;

    Actor *m_mesh;
    Actor *m_light;

    Material *m_material;

    ShaderNodeGraph *m_graph;
    ShaderBuilder *m_builder;

    CameraCtrl *m_controller;

    const UndoCommand *m_lastCommand;
};

#endif // MATERIALEDIT_H
