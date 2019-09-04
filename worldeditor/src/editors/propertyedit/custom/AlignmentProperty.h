#ifndef ALIGNMENTPROPERTY_H
#define ALIGNMENTPROPERTY_H

#include "Property.h"

class AlignmentProperty : public Property {
    Q_OBJECT
public:
    AlignmentProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option);

    bool setEditorData (QWidget *editor, const QVariant &data);

    QVariant editorData (QWidget *editor);

protected slots:
    void onDataChanged (int data);

};

#endif // ALIGNMENTPROPERTY_H
