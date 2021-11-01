#ifndef COMPONENTPROPERTY_H
#define COMPONENTPROPERTY_H

#include "Property.h"

class SceneComponent;

class ComponentProperty : public Property {
    Q_OBJECT
public:
    explicit ComponentProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

private slots:
    void onComponentChanged(const SceneComponent &component);

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // COMPONENTPROPERTY_H
