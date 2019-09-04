#ifndef AXISESPROPERTY_H
#define AXISESPROPERTY_H

#include "Property.h"

class AxisesProperty : public Property {
    Q_OBJECT
public:
    AxisesProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option);

    bool setEditorData (QWidget *editor, const QVariant &data);

    QVariant editorData (QWidget *editor);

protected slots:
    void onDataChanged (int data);

};

#endif // AXISESPROPERTY_H
