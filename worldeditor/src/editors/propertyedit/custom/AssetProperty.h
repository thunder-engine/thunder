#ifndef TEMPLATEPROPERTY_H
#define TEMPLATEPROPERTY_H

#include "Property.h"

class TemplateProperty : public Property {
    Q_OBJECT
public:
    explicit TemplateProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

public slots:
    void onAssetChanged(const QString &uuid);

private:
    QVariant value(int role = Qt::UserRole) const override;
    void setValue(const QVariant &value) override;

    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // TEMPLATEPROPERTY_H
