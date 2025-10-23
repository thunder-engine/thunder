#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

#include <QMenu>

class Prefab;
class ObjectController;
class WorldObserver;

class QToolButton;
class QPushButton;

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public AssetEditor {
    Q_OBJECT

public:
    explicit SceneComposer(QWidget *parent = nullptr);
    ~SceneComposer();

    VariantMap saveState() override;
    void restoreState(const VariantMap &data) override;

    void backupScenes();
    void restoreBackupScenes();

    void takeScreenshot();

    QMenu *objectContextMenu(Object *object) override;

    QWidget *propertiesWidget(QWidget *parent) override;

    std::list<QWidget *> createActionWidgets(Object *object, QWidget *parent) const override;

private slots:
    void onScreenshot(QImage image);

    void onActivated() override;

    void onNewAsset() override;
    void onSave() override;
    void onSaveAs() override;

    void onUpdated() override;

    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onChangeSnap();

    void onShowToolPanel(QWidget *widget);

    void onSelectionChanged(std::list<Object *> objects);

    void onObjectCreate(TString type) override;
    void onObjectsSelected(Object::ObjectList objects, bool force) override;
    void onObjectsDeleted(Object::ObjectList objects) override;

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

    void onDrop(QDropEvent *event) override;
    void onDragEnter(QDragEnterEvent *event) override;
    void onDragMove(QDragMoveEvent *event) override;
    void onDragLeave(QDragLeaveEvent *event) override;

    void onDropMap(QString name, bool additive);

    void onLocal(bool flag);

    void onSetActiveScene();

    void onRepickSelected();

    void onCreateActor();
    void onActorDelete();
    void onActorDuplicate();
    void onPrefabIsolate();
    void onPrefabUnpack();
    void onPrefabUnpackCompletely();
    void onReloadPrefab();

    void onSaveIsolated();

    void onSaveAll();

    void onRemoveScene();
    void onDiscardChanges();

    void onDeleteComponent();

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path = TString()) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    StringList suffixes() const override;

    StringList componentGroups() const override;

    bool loadScene(const TString &path, bool additive);
    void saveScene(const TString &path, Scene *scene);
    void saveSceneAs(Scene *scene);

    void enterToIsolation(AssetConverterSettings *settings);
    void quitFromIsolation();

    Prefab *loadPrefab();

    QAction *createAction(QMenu &menu, const QString &name, const char *member, bool single, const QKeySequence &shortcut = 0);

private:
    Ui::SceneComposer *ui;

    QMenu m_actorMenu;
    QMenu m_sceneMenu;

    ObjectController *m_controller;

    WorldObserver *m_worldObserver;

    QList<ByteArray> m_backupScenes;

    QMap<uint32_t, AssetConverterSettings *> m_sceneSettings;

    QList<QPushButton *> m_toolButtons;

    QList<QAction *> m_objectActions;
    QList<QAction *> m_prefabActions;
    QAction *m_activeSceneAction;

    VariantMap m_isolationBackState;

    AssetConverterSettings *m_isolationSettings;

    World *m_isolationWorld;
    Scene *m_isolationScene;

    QToolButton *m_componentButton;

    QWidget *m_activeToolPanel;
};

#endif // SCENECOMPOSER_H
