#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <editor/editorgadget.h>

class QAbstractItemModel;
class PropertyFilter;

class Object;
class NextModel;
class PropertyModel;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public EditorGadget {
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parent = nullptr);

    ~PropertyEditor();

    void onObjectsSelected(const Object::ObjectList &objects) override;

    QAbstractItemModel *model();

    void setGroup(const QString &group);

    void setTopWidget(QWidget *widget);

    AssetEditor *currentEditor() const;
    void setCurrentEditor(AssetEditor *editor) override;

protected:
    void updatePersistent(const QModelIndex &index);

    void updateAndExpand();

protected slots:
    void onUpdated() override;

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override;

    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    void changeEvent(QEvent *event) override;

private:
    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    AssetEditor *m_editor;

    QWidget *m_topWidget;

    NextModel *m_nextModel;

    Object *m_item;

};

#endif // PROPERTYEDITOR_H
