#ifndef OBJECTSELECT_H
#define OBJECTSELECT_H

#include <editor/propertyedit.h>
#include <editor/assetmanager.h>

#include <components/actor.h>
#include <components/component.h>

#include "editors/objecthierarchy/objecthierarchymodel.h"

namespace Ui {
    class ObjectSelect;
}

class ObjectSelect : public PropertyEdit {
    Q_OBJECT

public:
    explicit ObjectSelect(QWidget *parent = nullptr);
    ~ObjectSelect();

    QVariant data() const override;
    void setData(const QVariant &data) override;

    void setObjectData(const ObjectData &data);
    void setTemplateData(const Template &data);

private slots:
    void onDialog();

    void onComponentSelected(Object *object);
    void onAssetSelected(QString asset);

private:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    Ui::ObjectSelect *ui;

    ObjectData m_objectData;
    Template m_templateData;

    QAction *m_icon;

    bool m_asset;

};

#endif // OBJECTSELECT_H
