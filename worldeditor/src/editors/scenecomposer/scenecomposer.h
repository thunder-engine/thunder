#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

class NextObject;

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

signals:
    void hierarchyCreated(Object *root);
    void hierarchyUpdated();

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

private:
    void newAsset() override;
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    QStringList suffixes() const override;

private:
    Ui::SceneComposer *ui;

    ByteArray m_backupScene;

    NextObject *m_properties;

};

#endif // SCENECOMPOSER_H
