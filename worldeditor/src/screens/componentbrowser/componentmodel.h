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
