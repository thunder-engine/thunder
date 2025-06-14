#ifndef NEXTMODEL_H
#define NEXTMODEL_H

#include "propertymodel.h"

#include <variant.h>

class Object;

class NextModel : public PropertyModel {
    Q_OBJECT

public:
    explicit NextModel(QObject* parent = nullptr);

    void addItem(Object *propertyObject);

signals:
    void propertyChanged(std::list<Object *> objects, const QString property, Variant value);

private:
    void updateDynamicProperties(Property *parent, Object *propertyObject);

};

#endif
