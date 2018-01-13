#ifndef COMPONENTMODEL_H
#define COMPONENTMODEL_H

#include "baseobjectmodel.h"

#include <patterns/asingleton.h>

class Engine;

class ComponentModel : public BaseObjectModel, public ASingleton<ComponentModel> {
    Q_OBJECT

public:
    void                    init            (Engine *engine);

    int                     columnCount     (const QModelIndex &parent) const;

    QVariant                headerData      (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant                data            (const QModelIndex &index, int role = Qt::DisplayRole) const;

protected slots:
    void                    update          ();

protected:
    friend class ASingleton<ComponentModel>;

    ComponentModel          ();
    ~ComponentModel         () {}

protected:
    Engine                *m_pEngine;

};

#endif // COMPONENTMODEL_H
