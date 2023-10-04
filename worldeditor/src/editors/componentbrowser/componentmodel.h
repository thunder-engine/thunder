#ifndef COMPONENTMODEL_H
#define COMPONENTMODEL_H

#include "editors/baseobjectmodel/baseobjectmodel.h"

class Engine;

class ComponentModel : public BaseObjectModel {
    Q_OBJECT

public:
    static ComponentModel *instance();

    static void destroy();

    int columnCount(const QModelIndex &) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    void update();

private:
    ComponentModel();
    ~ComponentModel() {}

    static ComponentModel  *m_pInstance;

};

#endif // COMPONENTMODEL_H
