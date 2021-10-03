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
    bool m_Modified;

    Ui::MaterialEdit *ui;

    Actor *m_pMesh;
    Actor *m_pLight;

    Material *m_pMaterial;

    ShaderBuilder *m_pBuilder;

    QMenu *m_pCreateMenu;
    QWidgetAction *m_pAction;

    ComponentBrowser *m_pBrowser;

private slots:
    void onComponentSelected(const QString &path);

    void onNodesSelected(const QVariant &);
    void onUpdateTemplate(bool update = true);

    void on_actionPlane_triggered();
    void on_actionCube_triggered();
    void on_actionSphere_triggered();

    void on_schemeWidget_customContextMenuRequested(const QPoint &);
};

#endif // MATERIALEDIT_H
