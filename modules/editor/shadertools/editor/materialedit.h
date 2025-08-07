#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <editor/asseteditor.h>

#include <components/meshrender.h>

#include "shadercodedialog.h"

class ShaderBuilder;
class ShaderGraph;
class CameraController;

class UndoCommand;

class MaterialProxy;

namespace Ui {
    class MaterialEdit;
}

class MaterialEdit : public AssetEditor {
    Q_OBJECT
    
public:
    MaterialEdit();
    ~MaterialEdit();

    void onGraphUpdated();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

    void on_actionCode_triggered();

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void readSettings();
    void writeSettings();

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    void changeMesh(Mesh *mesh);
    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    StringList suffixes() const override;

private:
    ShaderCodeDialog m_codeDlg;

    Ui::MaterialEdit *ui;

    Actor *m_mesh;
    Actor *m_light;

    Material *m_material;

    ShaderGraph *m_graph;
    ShaderBuilder *m_builder;

    CameraController *m_controller;

    MaterialProxy *m_proxy;

};

#endif // MATERIALEDIT_H
