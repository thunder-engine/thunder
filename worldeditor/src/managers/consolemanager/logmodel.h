#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include <stdint.h>

class LogModel : public QAbstractItemModel {
public:
    LogModel            ();

    void                addRecord       (uint8_t type, const QString &str);

    void                clear           ();

    int                 columnCount     (const QModelIndex &parent) const;

    QVariant            data            (const QModelIndex &index, int role) const;

    int                 rowCount        (const QModelIndex &parent) const;

    QModelIndex         index           (int row, int column, const QModelIndex &parent) const;

    QModelIndex         parent          (const QModelIndex &index) const;

    Qt::ItemFlags       flags           (const QModelIndex &index) const;

protected:
    QStringList         m_Records;
    QList<uint8_t>      m_Types;

    QIcon               m_Error;
    QIcon               m_Warning;
    QIcon               m_Info;

};

#endif // LOGMODEL_H
