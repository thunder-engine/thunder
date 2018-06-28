#include "Vector2DProperty.h"

Vector2DProperty::Vector2DProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {

    pX  = new Property("x", this, this);
    pY  = new Property("y", this, this);
}

QVariant Vector2DProperty::value(int role) const {
    QVariant data = Property::value(role);
    if (data.isValid() && role != Qt::UserRole) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("[ %1, %2 ]").
                        arg(pX->value(role).toString()).
                        arg(pY->value(role).toString());
            case Qt::EditRole:
                return tr("%1, %2").
                        arg(pX->value(role).toString()).
                        arg(pY->value(role).toString());
            default:
                break;
        };
    }
    return data;
}

void Vector2DProperty::setValue(const QVariant &value) {
    if (value.type() == QVariant::String) {
        QString v   = value.toString();
        QRegExp rx("([+-]?([0-9]*[\\.,])?[0-9]+(e[+-]?[0-9]+)?)");
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        int count   = 0;
        int pos     = 0;
        float x     = 0.0f, y = 0.0f;

        while ((pos = rx.indexIn(v, pos)) != -1) {
            if (count == 0)
                x = rx.cap(1).toDouble();
            else if (count == 1)
                y = rx.cap(1).toDouble();
            else if (count > 2)
                break;
            ++count;
            pos += rx.matchedLength();
        }
        pX  ->setProperty("x", x);
        pY  ->setProperty("y", y);
        Property::setValue(QVariant::fromValue(Vector2(x, y)));
    } else
        Property::setValue(value);
}

QSize Vector2DProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

float Vector2DProperty::x() const {
    return value().value<Vector2>().x;
}

void Vector2DProperty::setX(float value) {
    Property::setValue(QVariant::fromValue(Vector2(value, y())));
}

float Vector2DProperty::y() const {
    return value().value<Vector2>().y;
}

void Vector2DProperty::setY(float value) {
    Property::setValue(QVariant::fromValue(Vector2(x(), value)));
}
