#ifndef VECTOR2DPROPERTY_H
#define VECTOR2DPROPERTY_H

#include "Property.h"

#include <amath.h>
Q_DECLARE_METATYPE(Vector2)

class Vector2DProperty : public Property {
    Q_OBJECT

    Q_PROPERTY(float x READ x WRITE setX DESIGNABLE true USER true)
    Q_PROPERTY(float y READ y WRITE setY DESIGNABLE true USER true)

public:
    Vector2DProperty                    (const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);

    QVariant            value           (int role = Qt::UserRole) const;
    void                setValue        (const QVariant& value);

    float               x               () const;
    void                setX            (float value);

    float               y               () const;
    void                setY            (float value);

private:
    Property           *pX;
    Property           *pY;

};

#endif // VECTOR2DPROPERTY_H
