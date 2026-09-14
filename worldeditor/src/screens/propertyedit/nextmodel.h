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
#ifndef NEXTMODEL_H
#define NEXTMODEL_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

#include <variant.h>
#include <object.h>

class Property;

class NextModel : public BaseObjectModel {
    Q_OBJECT

public:
    explicit NextModel(QObject* parent = nullptr);
    ~NextModel();

    void addObject(Object *propertyObject);
    void addObjects(const Object::ObjectList &propertyObjects);

    void clear();

signals:
    void propertyChanged(const Object::ObjectList &objects, const TString property, Variant value);

private:
    void updateDynamicProperties(Property *parent, Object *propertyObject);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

};

QString fromCamelCase(const TString &s);

#endif
