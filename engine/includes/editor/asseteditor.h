#ifndef ASSETEDITOR_H
#define ASSETEDITOR_H

#include <QWidget>

#include <engine.h>

class AssetConverterSettings;
class QMenu;

class UndoStack;

class ENGINE_EXPORT AssetEditor : public QWidget {
    Q_OBJECT

public:
    AssetEditor();
    ~AssetEditor();

    std::list<AssetConverterSettings *> &openedDocuments();

    virtual void loadAsset(AssetConverterSettings *settings) = 0;

    virtual void loadData(const Variant &data, const TString &suffix);

    virtual bool isSingleInstance() const;

    virtual bool isCopyActionAvailable() const;
    virtual bool isPasteActionAvailable() const;

    virtual AssetEditor *createInstance();

    virtual StringList suffixes() const = 0;

    virtual StringList componentGroups() const;

    virtual QMenu *objectContextMenu(Object *object);

    virtual QWidget *propertiesWidget(QWidget *parent);

    UndoStack *undoRedo() const;

    virtual std::list<QWidget *> createActionWidgets(QObject *object, QWidget *parent) const;
    virtual std::list<QWidget *> createActionWidgets(Object *object, QWidget *parent) const;

    virtual VariantMap saveState();
    virtual void restoreState(const VariantMap &data);

    bool checkSave();

    virtual bool allowSaveAs() const;

signals:
    void updated();

    void objectsHierarchyChanged(Object *root);

    void objectsSelected(Object::ObjectList objects);

    void objectsChanged(const Object::ObjectList &objects, TString property, const Variant &value);

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

    virtual void onObjectCreate(TString type);
    virtual void onObjectsSelected(Object::ObjectList objects, bool force);
    virtual void onObjectsDeleted(Object::ObjectList objects);

    virtual void onDrop(QDropEvent *event);
    virtual void onDragEnter(QDragEnterEvent *event);
    virtual void onDragMove(QDragMoveEvent *event);
    virtual void onDragLeave(QDragLeaveEvent *event);

    virtual void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value);

protected:
    virtual void setModified(bool flag);

    virtual bool isModified() const = 0;

    virtual void saveAsset(const TString &path = TString());

    int closeAssetDialog();

protected:
    std::list<AssetConverterSettings *> m_settings;

    UndoStack *m_undoRedo;
};

#endif // ASSETEDITOR_H
