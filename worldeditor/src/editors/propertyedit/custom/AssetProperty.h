#ifndef TEMPLATEPROPERTY_H
#define TEMPLATEPROPERTY_H

#include "Property.h"

#include <engine.h>

class TemplateProperty : public Property {
    Q_OBJECT
public:
    TemplateProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QVariant value (int role = Qt::UserRole) const;
    void setValue (const QVariant &value);

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option);
    bool setEditorData (QWidget *editor, const QVariant &data);
    QVariant editorData (QWidget *editor);

    QSize sizeHint (const QSize &size) const;

public slots:
    void onAssetChanged (const QString &uuid);

};

#endif // TEMPLATEPROPERTY_H
