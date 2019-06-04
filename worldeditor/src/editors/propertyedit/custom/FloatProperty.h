#ifndef FLOATPROPERTY_H
#define FLOATPROPERTY_H

#include "Property.h"

class FloatProperty : public Property {
    Q_OBJECT

public:
    FloatProperty       (const QString& name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem& option);

    bool                setEditorData   (QWidget *editor, const QVariant &data);

    QVariant            editorData      (QWidget *editor);

protected slots:
    void                onDataChanged   ();

};

#endif // FLOATPROPERTY_H
