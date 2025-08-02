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

    void addItem(Object *propertyObject);

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
