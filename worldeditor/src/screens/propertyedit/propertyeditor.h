#ifndef QPROPERTYEDITORWIDGET_H
#define QPROPERTYEDITORWIDGET_H

#include <editor/editorgadget.h>

class QAbstractItemModel;
class PropertyFilter;

class Object;
class NextObject;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public EditorGadget {
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parent = nullptr);

    ~PropertyEditor();

    QObject *object() const;

    void onItemsSelected(QList<QObject *> items) override;

    void onObjectsSelected(QList<Object *> objects) override;

    QAbstractItemModel *model();

    void setGroup(const QString &group);

signals:
    void commited();
    void reverted();

protected:
    void setCurrentEditor(AssetEditor *editor) override;

    void updatePersistent(const QModelIndex &index);

    void addObject(QObject *propertyObject, const QString &name = QString(), QObject *parent = nullptr);

protected slots:
    void onUpdated() override;

    void onObjectsChanged(QList<Object *> objects, const QString property, Variant value) override;

    void onStructureChanged();

    void onSettingsUpdated();

    void on_commitButton_clicked();
    void on_revertButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    void changeEvent(QEvent *event) override;

    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    QObject *m_propertyObject;

    NextObject *m_nextObject;

    AssetEditor *m_editor;

};

#endif
