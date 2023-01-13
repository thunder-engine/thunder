#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H

#include <editor/property.h>

class BoolProperty : public Property {
    Q_OBJECT

public:
    explicit BoolProperty(const QString& name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged(int data);

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // BOOLPROPERTY_H
