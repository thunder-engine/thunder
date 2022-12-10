#ifndef OBJECTPROPERTY_H
#define OBJECTPROPERTY_H

#include "../propertyedit/custom/Property.h"

class ObjectData;

class ObjectProperty : public Property {
    Q_OBJECT
public:
    explicit ObjectProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

private slots:
    void onValueChanged();

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // OBJECTPROPERTY_H
