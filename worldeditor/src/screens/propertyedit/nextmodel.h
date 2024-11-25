#ifndef NEXTMODEL_H
#define NEXTMODEL_H

#include "propertymodel.h"

class Object;

class NextModel : public PropertyModel {
    Q_OBJECT

public:
    explicit NextModel(QObject* parent = nullptr);

    void addItem(Object *propertyObject);

private:
    void updateDynamicProperties(Property *parent, Object *propertyObject);

};

#endif
