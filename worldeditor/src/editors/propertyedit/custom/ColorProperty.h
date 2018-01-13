#ifndef COLORPROPERTY_H
#define COLORPROPERTY_H

#include "Property.h"

#include <amath.h>

class ColorProperty : public Property {
    Q_OBJECT

public:
    ColorProperty                       (const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);

    QVariant            value           (int role = Qt::UserRole) const;
    void                setValue        (const QVariant& value);

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem &option);

    bool                setEditorData   (QWidget *editor, const QVariant &data);
    QVariant            editorData      (QWidget *editor);

    QSize               sizeHint        (const QSize& size) const;

private slots:
    void                onColorChanged  (const QString &color);

};

#endif // COLORPROPERTY_H
