#ifndef TEMPLATEPROPERTY_H
#define TEMPLATEPROPERTY_H

#include "Property.h"

#include <engine.h>

#include "converters/converter.h"

class TemplateProperty : public Property {
    Q_OBJECT
public:
    TemplateProperty                        (const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0);

    QVariant            value               (int role = Qt::UserRole) const;
    void                setValue            (const QVariant &value);

    QWidget            *createEditor        (QWidget *parent, const QStyleOptionViewItem &option);
    bool                setEditorData       (QWidget *editor, const QVariant &data);
    QVariant            editorData          (QWidget *editor);

    QSize               sizeHint            (const QSize &size) const;

public slots:
    void                onAssetChanged      (IConverterSettings *settings);

};

#endif // TEMPLATEPROPERTY_H
