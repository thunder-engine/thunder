#ifndef AXISESPROPERTY_H
#define AXISESPROPERTY_H

#include "Property.h"

class AxisesProperty : public Property {
    Q_OBJECT
public:
    explicit AxisesProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged(int data);

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // AXISESPROPERTY_H
