#ifndef VECTOR4DPROPERTY_H
#define VECTOR4DPROPERTY_H

#include "Property.h"

#include <amath.h>
Q_DECLARE_METATYPE(Vector4)

class Vector4DProperty : public Property {
    Q_OBJECT

public:
    Vector4DProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QVariant value(int role = Qt::UserRole) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &);
    bool setEditorData(QWidget *editor, const QVariant &data);
    QVariant editorData(QWidget *editor);

    QSize sizeHint(const QSize &size) const;

protected slots:
    void onDataChanged(const QVariant &data);

};

#endif // VECTOR4DPROPERTY_H
