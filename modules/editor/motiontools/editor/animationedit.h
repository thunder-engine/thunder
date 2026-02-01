#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMenu>

#include <editor/asseteditor.h>

class AnimationControllerGraph;
class AnimationStateMachine;
class AssetConverter;

class CameraController;

class Animator;

class AnimationProxy;

class QToolButton;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public AssetEditor {
    Q_OBJECT

public:
    AnimationEdit();
    ~AnimationEdit();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onAddVariable(QAction *action);

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

    void onObjectsSelected(const Object::ObjectList &objects);

    void onRenameVariable();
    void onDeleteVariable();

private:
    QWidget *propertiesWidget() override;

    QMenu *propertyContextMenu(Object *object, const TString &property) override;

    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    bool isModified() const override;

    StringList suffixes() const override;

    void changeEvent(QEvent *event) override;

private:
    friend class AnimationProxy;

    Ui::AnimationEdit *ui;

    AnimationControllerGraph *m_graph;
    AssetConverter *m_assetConverter;

    CameraController *m_controller;

    Actor *m_light;

    Animator *m_animator;

    AnimationProxy *m_proxy;

    QToolButton *m_variableButton;

    QMenu m_variablesMenu;

};

#endif // ANIMATIONEDIT_H
