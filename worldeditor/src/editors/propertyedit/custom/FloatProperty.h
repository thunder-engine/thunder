#ifndef FLOATPROPERTY_H
#define FLOATPROPERTY_H

#include <editor/property.h>

class FloatProperty : public Property {
    Q_OBJECT

public:
    explicit FloatProperty(const QString& name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged();

protected:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // FLOATPROPERTY_H
