#ifndef COLORPROPERTY_H
#define COLORPROPERTY_H

#include "Property.h"

class ColorProperty : public Property {
    Q_OBJECT

public:
    explicit ColorProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

private slots:
    void onColorChanged(const QString &color);

private:
    QVariant value(int role = Qt::UserRole) const override;
    void setValue(const QVariant &value) override;

    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;
    QVariant editorData(QWidget *editor) override;

};

#endif // COLORPROPERTY_H
