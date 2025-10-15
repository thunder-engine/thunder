#ifndef OBJECTSELECT_H
#define OBJECTSELECT_H

#include <QLineEdit>

#include <editor/propertyedit.h>
#include <editor/assetmanager.h>

#include <components/actor.h>
#include <components/component.h>

#include "screens/objecthierarchy/objecthierarchymodel.h"

namespace Ui {
    class ObjectSelect;
}

class LineEdit : public QLineEdit {
    Q_OBJECT

public:
    LineEdit(QWidget *parent) :
            QLineEdit(parent) {
    }

signals:
    void dragEnter(QDragEnterEvent *e);
    void drop(QDropEvent *e);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override { emit dragEnter(e); }
    void dropEvent(QDropEvent *e) override { emit drop(e); }
};

class ObjectSelect : public PropertyEdit {
    Q_OBJECT

public:
    explicit ObjectSelect(QWidget *parent = nullptr);
    ~ObjectSelect();

    Variant data() const override;
    void setData(const Variant &data) override;

    void setType(const TString &type);

private:
    void setObjectData(const Variant &data);
    void setTemplateData(const Variant &data);

    void setObject(Object *object, const TString &name) override;

private slots:
    void onDialog();

    void onComponentSelected(Object *object);
    void onAssetSelected(QString asset);

    void onDragEnter(QDragEnterEvent *event);
    void onDrop(QDropEvent *event);

private:
    Ui::ObjectSelect *ui;

    Variant m_data;

    TString m_type;

    Scene *m_scene = nullptr;

    QAction *m_icon;

};

#endif // OBJECTSELECT_H
