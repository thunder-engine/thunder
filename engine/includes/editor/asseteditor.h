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

    virtual bool isCopyActionAvailable() const;
    virtual bool isPasteActionAvailable() const;

    virtual AssetEditor *createInstance();

    virtual QStringList suffixes() const = 0;

    virtual QStringList componentGroups() const;

    virtual QMenu *objectContextMenu(Object *object);

    virtual QWidget *propertiesWidget();

    virtual std::list<QWidget *> createActionWidgets(QObject *object, QWidget *parent) const;
    virtual std::list<QWidget *> createActionWidgets(Object *object, QWidget *parent) const;

    virtual VariantMap saveState();
    virtual void restoreState(const VariantMap &data);

    bool checkSave();

    virtual bool allowSaveAs() const;

signals:
    void updated();

    void itemsHierarchyChanged(QObject *root);
    void objectsHierarchyChanged(Object *root);

    void itemsSelected(std::list<QObject *> items);
    void objectsSelected(Object::ObjectList objects);

    void itemsChanged(const std::list<QObject *> &objects, QString property, const QVariant &value);
    void objectsChanged(const Object::ObjectList &objects, QString property, const Variant &value);

    void copyPasteChanged();

    void dropAsset(QString path);

public slots:
    virtual void onActivated();

    virtual void onCutAction();
    virtual void onCopyAction();
    virtual void onPasteAction();

    virtual void onNewAsset();
    virtual void onSave();
    virtual void onSaveAs();

    virtual void onUpdated();

    virtual void onObjectCreate(QString type);
    virtual void onObjectsSelected(Object::ObjectList objects, bool force);
    virtual void onObjectsDeleted(Object::ObjectList objects);

    virtual void onDrop(QDropEvent *event);
    virtual void onDragEnter(QDragEnterEvent *event);
    virtual void onDragMove(QDragMoveEvent *event);
    virtual void onDragLeave(QDragLeaveEvent *event);

    virtual void onObjectsChanged(const Object::ObjectList &objects, QString property, const Variant &value);

protected:
    virtual void setModified(bool flag);

    virtual bool isModified() const = 0;

    virtual void saveAsset(const QString &path = QString());

    int closeAssetDialog();

protected:
    QList<AssetConverterSettings *> m_settings;

};

#endif // ASSETEDITOR_H
