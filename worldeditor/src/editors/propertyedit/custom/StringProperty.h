#ifndef STRINGPROPERTY_H
#define STRINGPROPERTY_H

#include "Property.h"

class StringProperty : public Property {
    Q_OBJECT

public:
    StringProperty      (const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);
    ~StringProperty     ();

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem& option);

    bool                setEditorData   (QWidget *editor, const QVariant &data);

    QVariant            editorData      (QWidget *editor);

protected slots:
    void                onDataChanged   (const QString &data);

};

#endif // STRINGPROPERTY_H
