#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

#include <QMenu>

class NextObject;
class ObjectCtrl;

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public AssetEditor {
    Q_OBJECT

public:
    explicit SceneComposer(QWidget *parent = nullptr);
    ~SceneComposer();

    void setEngine(Engine *engine);

    VariantList saveState();
    void restoreState(const VariantList &state);

    void backupScene();
    void restoreBackupScene();

    void takeScreenshot();

    QString path() const;

    QMenu *contextMenu();

signals:
    void hierarchyCreated(Object *root);

    void renameItem();

    void createComponent(QString);

    void itemsSelected(const Object::ObjectList &objects);

public slots:
    void onSelectActors(Object::ObjectList objects);
    void onRemoveActors(Object::ObjectList objects);
    void onUpdated();
    void onParentActors(Object::ObjectList objects, Object *parent);
    void onFocusActor(Object *actor);

private slots:
    void onLocal(bool flag);

    void onRepickSelected();

    void onItemsSelected(const Object::ObjectList &objects);

    void onCreateActor();
    void onItemDuplicate();
    void onItemDelete();
    void onPrefabIsolate();
    void onPrefabUnpack();
    void onPrefabUnpackCompletely();

    void onSaveIsolated();

    void onObjectMenuAboutToShow();

    void onDropMap(QString name, bool additive);

    void onActivated() override;

private:
    void newAsset() override;
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    QStringList suffixes() const override;

    bool loadMap(QString path, bool additive);

    void enterToIsolation(AssetConverterSettings *settings);
    void quitFromIsolation();

    QAction *createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut = 0);

private:
    Ui::SceneComposer *ui;

    QMenu m_contentMenu;

    VariantList m_isolationBackState;

    QList<ByteArray> m_backupScenes;

    QList<QAction *> m_prefabActions;

    AssetConverterSettings *m_isolationSettings;

    NextObject *m_properties;

    ObjectCtrl *m_controller;

    Scene *m_isolationScene;

    Engine *m_engine;

};

#endif // SCENECOMPOSER_H
