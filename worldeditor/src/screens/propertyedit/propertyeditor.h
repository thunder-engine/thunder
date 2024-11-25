#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

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

    void onItemsSelected(QList<QObject *> items) override;

    void onObjectsSelected(QList<Object *> objects) override;

    QAbstractItemModel *model();

    void setGroup(const QString &group);

    void setTopWidget(QWidget *widget);

    QList<QWidget *> getActions(QObject *object, const QString &name);

protected:
    void setCurrentEditor(AssetEditor *editor) override;

    void updatePersistent(const QModelIndex &index);

    void updateAndExpand();

protected slots:
    void onUpdated() override;

    void onObjectsChanged(QList<Object *> objects, const QString property, Variant value) override;

    void onStructureChanged();

    void on_lineEdit_textChanged(const QString &arg1);

private:
    void changeEvent(QEvent *event) override;

    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    QObject *m_propertyObject;

    NextObject *m_nextObject;

    AssetEditor *m_editor;

    QWidget *m_topWidget;

};

#endif // PROPERTYEDITOR_H
