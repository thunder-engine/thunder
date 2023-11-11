#ifndef ASSETEDITOR_H
#define ASSETEDITOR_H

#include <QWidget>

#include <engine.h>

class AssetConverterSettings;
class QMenu;

class ENGINE_EXPORT AssetEditor : public QWidget {
    Q_OBJECT

public:
    AssetEditor();
    ~AssetEditor();

    QList<AssetConverterSettings *> &openedDocuments();

    virtual void loadAsset(AssetConverterSettings *settings) = 0;

    virtual void loadData(const Variant &data, const QString &suffix);

    virtual bool isSingleInstance() const;

    virtual AssetEditor *createInstance();

    virtual QStringList suffixes() const = 0;

    virtual QStringList componentGroups() const;

    virtual QMenu *objectMenu(Object *object);

    virtual VariantList saveState();
    virtual void restoreState(const VariantList &list);

    bool checkSave();

signals:
    void updated();

    void itemsHierarchyChanged(QObject *root);
    void objectsHierarchyChanged(Object *root);

    void itemsSelected(QList<QObject *> items);
    void objectsSelected(QList<Object *> objects);

    void itemsChanged(const QList<QObject *> &objects, QString property, const QVariant &value);
    void objectsChanged(const QList<Object *> &objects, QString property, const Variant &value);

    void dropAsset(QString path);

public slots:
    virtual void onActivated();

    virtual void onNewAsset();
    virtual void onSave();
    virtual void onSaveAs();

    virtual void onUpdated();

    virtual void onObjectCreate(QString type);
    virtual void onObjectsSelected(QList<Object *> objects, bool force);
    virtual void onObjectsDeleted(QList<Object *> objects);

    virtual void onDrop(QDropEvent *event);
    virtual void onDragEnter(QDragEnterEvent *event);
    virtual void onDragMove(QDragMoveEvent *event);
    virtual void onDragLeave(QDragLeaveEvent *event);

    virtual void onObjectsChanged(const QList<Object *> &objects, QString property, const Variant &value);

protected:
    virtual void setModified(bool flag);

    virtual bool isModified() const = 0;

    virtual void saveAsset(const QString &path = QString());

    int closeAssetDialog();

protected:
    QList<AssetConverterSettings *> m_settings;

};

#endif // ASSETEDITOR_H
