#ifndef INTEGERPROPERTY_H
#define INTEGERPROPERTY_H

#include "Property.h"

class IntegerProperty : public Property {
    Q_OBJECT

public:
    IntegerProperty     (const QString& name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem& option);

    bool                setEditorData   (QWidget *editor, const QVariant &data);

    QVariant            editorData      (QWidget *editor);

protected slots:
    void                onDataChanged   ();

};

#endif // INTEGERPROPERTY_H
