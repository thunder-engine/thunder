#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H

#include "Property.h"

class BoolProperty : public Property {
    Q_OBJECT

public:
    BoolProperty        (const QString& name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);
    ~BoolProperty       ();

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem& option);

    bool                setEditorData   (QWidget *editor, const QVariant &data);

    QVariant            editorData      (QWidget *editor);

protected slots:
    void                onDataChanged   (int data);

};

#endif // BOOLPROPERTY_H
