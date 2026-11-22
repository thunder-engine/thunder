#ifndef LEGACYMATERIALEDIT_H
#define LEGACYMATERIALEDIT_H

#include <editor/legacyasseteditor.h>

#include <components/meshrender.h>

#include "shadercodedialog.h"

class ShaderBuilder;
class ShaderGraph;
class LegacyCameraController;

class MaterialProxy;

namespace Ui {
    class LegacyMaterialEdit;
}

class LegacyMaterialEdit : public LegacyAssetEditor {
    Q_OBJECT
    
public:
    LegacyMaterialEdit();
    ~LegacyMaterialEdit();

    void onGraphUpdated();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onObjectsChanged(const TString &property, const Variant &value) override;

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

    StringList suffixes() const override;
    TString assetType() const override;

    Object::ObjectList selected() const override;

private:
    ShaderCodeDialog m_codeDlg;

    Ui::LegacyMaterialEdit *ui;

    Actor *m_mesh;
    Actor *m_light;

    Material *m_material;

    ShaderGraph *m_graph;
    ShaderBuilder *m_builder;

    LegacyCameraController *m_controller;

    MaterialProxy *m_proxy;

};

#endif // LEGACYMATERIALEDIT_H
