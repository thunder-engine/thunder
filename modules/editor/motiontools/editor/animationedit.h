#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMenu>

#include <editor/legacyasseteditor.h>

class AnimationControllerBuilder;
class AnimationControllerGraph;
class AnimationStateMachine;

class LegacyCameraController;
class Armature;
class Animator;

class AnimationProxy;

class QToolButton;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public LegacyAssetEditor {
    Q_OBJECT

public:
    AnimationEdit();
    ~AnimationEdit();

    void onGraphUpdated();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onAddVariable(QAction *action);

    void onObjectsChanged(const TString &property, const Variant &value) override;

    void onSelectionChanged();

    void onRenameVariable();
    void onDeleteVariable();

private:
    void readSettings();
    void writeSettings();

    QWidget *propertiesWidget() override;

    QMenu *propertyContextMenu(Object *object, const TString &property) override;

    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    StringList suffixes() const override;
    TString assetType() const override;

    void changeEvent(QEvent *event) override;
    void timerEvent(QTimerEvent *) override;

    Object::ObjectList selected() const override;

private:
    friend class AnimationProxy;

    Ui::AnimationEdit *ui;

    AnimationControllerGraph *m_graph;
    AnimationControllerBuilder *m_builder;

    LegacyCameraController *m_controller;

    Actor *m_light;

    Animator *m_animator;

    Armature *m_armature;

    AnimationStateMachine *m_stateMachine;

    AnimationProxy *m_proxy;

    QToolButton *m_variableButton;

    QMenu m_variablesMenu;

};

#endif // ANIMATIONEDIT_H
