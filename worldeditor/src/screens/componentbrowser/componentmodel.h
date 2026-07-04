#ifndef COMPONENTMODEL_H
#define COMPONENTMODEL_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

class Engine;

class ComponentModel : public BaseObjectModel {
    Q_OBJECT

public:
    static ComponentModel *instance();

public slots:
    void update();

private:
    ComponentModel();
    ~ComponentModel() {}

    int columnCount(const QModelIndex &) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // COMPONENTMODEL_H
