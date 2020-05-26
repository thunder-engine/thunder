#ifndef COMPONENTPROPERTY_H
#define COMPONENTPROPERTY_H

#include "Property.h"

#include <engine.h>

#include "converters/converter.h"

class SceneComponent;

class ComponentProperty : public Property {
    Q_OBJECT
public:
    ComponentProperty (const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option);
    bool setEditorData (QWidget *editor, const QVariant &data);
    QVariant editorData (QWidget *editor);

    QSize sizeHint (const QSize &size) const;

private slots:
    void onComponentChanged(const SceneComponent &component);

};

#endif // COMPONENTPROPERTY_H
