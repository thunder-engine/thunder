#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <editor/asseteditor.h>

#include <components/meshrender.h>

#include "shadercodedialog.h"

class ShaderBuilder;
class ShaderNodeGraph;
class CameraController;

class UndoCommand;

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

    void onGraphUpdated();

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

    void on_actionCode_triggered();

private:
    void readSettings();
    void writeSettings();

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    void changeMesh(Mesh *mesh);
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

    CameraController *m_controller;

    const UndoCommand *m_lastCommand;

    ShaderCodeDialog m_codeDlg;

};

#endif // MATERIALEDIT_H
