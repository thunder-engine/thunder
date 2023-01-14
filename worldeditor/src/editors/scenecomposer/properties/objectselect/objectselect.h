#ifndef OBJECTSELECT_H
#define OBJECTSELECT_H

#include <QWidget>

#include <components/actor.h>
#include <components/component.h>
#include <assetmanager.h>

#include "editors/objecthierarchy/objecthierarchymodel.h"

namespace Ui {
    class ObjectSelect;
}

class ObjectSelect : public QWidget {
    Q_OBJECT

public:
    explicit ObjectSelect(QWidget *parent = nullptr);
    ~ObjectSelect();

    QVariant data() const;

    void setObjectData(const ObjectData &data);
    void setTemplateData(const Template &data);

signals:
    void valueChanged();

private slots:
    void onDialog();

    void onComponentSelected(Object *object);
    void onAssetSelected(QString asset);

private:
    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    Ui::ObjectSelect *ui;

    ObjectData m_objectData;
    Template m_templateData;

    QAction *m_icon;

    bool m_asset;

};

#endif // OBJECTSELECT_H
