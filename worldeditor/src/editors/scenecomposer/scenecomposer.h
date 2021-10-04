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

    void setScene(Scene *scene);

    VariantList saveState() const;
    void restoreState(const VariantList &state);

    void backupScene();
    void restoreBackupScene();

    void takeScreenshot();

    QString path() const;

    QMenu *contextMenu();

signals:
    void hierarchyCreated(Object *root);
    void hierarchyUpdated();

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
    void onItemUnpack();
    void onItemUnpackAll();
    void onItemRename();

    void onAboutToShow();

private:
    void newAsset() override;
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    QStringList suffixes() const override;

    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

private:
    Ui::SceneComposer *ui;

    QMenu m_contentMenu;

    ByteArray m_backupScene;

    NextObject *m_properties;

    ObjectCtrl *m_controller;

    QList<QAction *> m_prefab;

};

#endif // SCENECOMPOSER_H
