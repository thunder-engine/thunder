#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

#include <QMenu>

class NextObject;
class ObjectController;
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

    void objectsChanged(const QList<Object *> &objects, QString property, Variant value);

public slots:
    void onSelectActors(QList<Object *> objects);
    void onRemoveActors(QList<Object *> objects);
    void onUpdated();
    void onParentActors(QList<Object *> objects, Object *parent, int position);
    void onFocusActor(Object *actor);
    void onItemDelete();

    void onMenuRequested(Object *object, const QPoint &point);

    void onDropMap(QString name, bool additive);

private slots:
    void onLocal(bool flag);

    void onSetActiveScene();

    void onRepickSelected();

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

    ObjectController *m_controller;

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
