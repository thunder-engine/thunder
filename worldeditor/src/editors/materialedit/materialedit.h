#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <editor/asseteditor.h>

#include <components/meshrender.h>

class ShaderBuilder;

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

    void onComponentSelected();

    void onNodesSelected(const QVariant &);
    void onSchemeUpdated();

    void onShowContextMenu(int node, int port, bool out);

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

    ShaderBuilder *m_builder;

    QMenu *m_createMenu;

    QObject *m_selectedItem;

    const UndoCommand *m_lastCommand;

    int m_node;
    int m_port;
    bool m_out;

};

#endif // MATERIALEDIT_H
