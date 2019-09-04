#ifndef STRINGPROPERTY_H
#define STRINGPROPERTY_H

#include "Property.h"

class StringProperty : public Property {
    Q_OBJECT

public:
    StringProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);
    ~StringProperty ();

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option);

    bool setEditorData (QWidget *editor, const QVariant &data);

    QVariant editorData (QWidget *editor);

    QSize sizeHint (const QSize& size) const;

protected slots:
    void onDataChanged   ();

};

#endif // STRINGPROPERTY_H
