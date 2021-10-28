#ifndef MATERIALEDIT_H
#define MATERIALEDIT_H

#include <editor/asseteditor.h>

#include <components/meshrender.h>

class ShaderBuilder;

class ComponentBrowser;

class QWidgetAction;
class QMenu;

namespace Ui {
    class MaterialEdit;
}

class MaterialEdit : public AssetEditor {
    Q_OBJECT
    
public:
    MaterialEdit();
    ~MaterialEdit();

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

    ComponentBrowser *m_browser;

    QObject *m_selectedItem;

    int m_node;
    int m_port;
    bool m_out;

    bool m_modified;

private slots:
    void onActivated() override;

    void onComponentSelected(const QString &path);

    void onNodesSelected(const QVariant &);
    void onUpdateTemplate(bool update = true);

    void onShowContextMenu(int node, int port, bool out);

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

};

#endif // MATERIALEDIT_H
