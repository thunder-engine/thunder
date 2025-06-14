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

    void onItemsSelected(std::list<QObject *> items) override;

    void onObjectsSelected(std::list<Object *> objects) override;

    QAbstractItemModel *model();

    void setGroup(const QString &group);

    void setTopWidget(QWidget *widget);

    std::list<QWidget *> getActions(QObject *object, QWidget *parent);
    std::list<QWidget *> getActions(Object *object, QWidget *parent);

protected:
    void setCurrentEditor(AssetEditor *editor) override;

    void updatePersistent(const QModelIndex &index);

    void updateAndExpand();

protected slots:
    void onUpdated() override;

    void onObjectsChanged(std::list<Object *> objects, const QString property, Variant value) override;

    void on_lineEdit_textChanged(const QString &arg1);

private:
    void changeEvent(QEvent *event) override;

    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    AssetEditor *m_editor;

    QWidget *m_topWidget;

    NextModel *m_nextModel;

    PropertyModel *m_propertyModel;

};

#endif // PROPERTYEDITOR_H
