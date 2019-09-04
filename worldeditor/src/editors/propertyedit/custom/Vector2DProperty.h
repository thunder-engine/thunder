#ifndef VECTOR2DPROPERTY_H
#define VECTOR2DPROPERTY_H

#include "Property.h"

#include <amath.h>
Q_DECLARE_METATYPE(Vector2)

class Vector2DProperty : public Property {
    Q_OBJECT

public:
    Vector2DProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QVariant value (int role = Qt::UserRole) const;

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &);
    bool setEditorData (QWidget *editor, const QVariant &data);
    QVariant editorData (QWidget *editor);

    QSize sizeHint (const QSize &size) const;

protected slots:
    void onDataChanged (const QVariant &data);

};

#endif // VECTOR2DPROPERTY_H
