/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
