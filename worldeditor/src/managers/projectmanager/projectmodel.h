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
#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractListModel>
#include <QImage>

#include <astring.h>

class ProjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    ProjectModel();

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    static void addProject(const TString &path);

protected:
    static StringList s_list;

    mutable std::map<QString, QImage> m_iconCache;
};

#endif // PROJECTMODEL_H
