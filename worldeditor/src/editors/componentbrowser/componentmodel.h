#ifndef COMPONENTMODEL_H
#define COMPONENTMODEL_H

#include "baseobjectmodel.h"

class Engine;

class ComponentModel : public BaseObjectModel {
    Q_OBJECT

public:
    static ComponentModel  *instance        ();

    static void             destroy         ();

    void                    init            (Engine *engine);

    int                     columnCount     (const QModelIndex &parent) const;

    QVariant                headerData      (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant                data            (const QModelIndex &index, int role = Qt::DisplayRole) const;

protected slots:
    void                    update          ();

private:
    ComponentModel          ();
    ~ComponentModel         () {}

    static ComponentModel  *m_pInstance;

protected:
    Engine                 *m_pEngine;

};

#endif // COMPONENTMODEL_H
