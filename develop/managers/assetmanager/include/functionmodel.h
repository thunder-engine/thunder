#ifndef FUNCTIONMODEL_H
#define FUNCTIONMODEL_H

#include "baseobjectmodel.h"

#include <patterns/asingleton.h>

class Engine;

class FunctionModel : public BaseObjectModel {
    Q_OBJECT

public:
    FunctionModel           (const QStringList &classes);
    ~FunctionModel          () {}

    int                     columnCount     (const QModelIndex &parent) const;

    QVariant                headerData      (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant                data            (const QModelIndex &index, int role = Qt::DisplayRole) const;

protected slots:
    void                    update          ();

protected:
    QStringList             m_Classes;

};

#endif // FUNCTIONMODEL_H
