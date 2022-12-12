#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

#include <QMenu>

class NextObject;
class ObjectCtrl;
class WorldObserver;

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public AssetEditor {
    Q_OBJECT

public:
    explicit SceneComposer(QWidget *parent = nullptr);
    ~SceneComposer();

    void init();

    VariantList saveState();
    void restoreState(const VariantList &state);

    void backupScenes();
    void restoreBackupScenes();

    void takeScreenshot();

    QString map() const;

    World *currentWorld() const;
    void worldUpdated(World *graph);

signals:
    void hierarchyCreated(Object *root);

    void renameItem();

    void createComponent(QString);

    void itemsSelected(const Object::ObjectList &objects);

    void itemsChanged(const Object::ObjectList &objects, QString property);

public slots:
    void onSelectActors(Object::ObjectList objects);
    void onRemoveActors(Object::ObjectList objects);
    void onUpdated();
    void onParentActors(Object::ObjectList objects, Object *parent, int position);
    void onFocusActor(Object *actor);
    void onItemDelete();

    void onMenuRequested(Object *object, const QPoint &point);

private slots:
    void onLocal(bool flag);

    void onSetActiveScene();

    void onRepickSelected();

    void onItemsSelected(const Object::ObjectList &objects);

    void onCreateActor();
    void onItemDuplicate();
    void onPrefabIsolate();
    void onPrefabUnpack();
    void onPrefabUnpackCompletely();

    void onSaveIsolated();
    void onSave() override;
    void onSaveAs() override;
    void onSaveAll();

    void onRemoveScene();
    void onDiscardChanges();
    void onNewAsset() override;

    void onDropMap(QString name, bool additive);

    void onActivated() override;

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    QStringList suffixes() const override;

    bool loadMap(QString path, bool additive);
    void saveMap(QString path, Scene *scene);

    void enterToIsolation(AssetConverterSettings *settings);
    void quitFromIsolation();

    QAction *createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut = 0);

private:
    Ui::SceneComposer *ui;

    QMenu m_actorMenu;
    QMenu m_sceneMenu;

    Object *m_menuObject;

    NextObject *m_properties;

    ObjectCtrl *m_controller;

    WorldObserver *m_worldObserver;

    QList<ByteArray> m_backupScenes;

    QMap<uint32_t, AssetConverterSettings *> m_sceneSettings;

    QList<QAction *> m_objectActions;
    QList<QAction *> m_prefabActions;
    QAction *m_activeSceneAction;

    VariantList m_isolationBackState;

    AssetConverterSettings *m_isolationSettings;

    World *m_isolationWorld;
    Scene *m_isolationScene;

};

#endif // SCENECOMPOSER_H
