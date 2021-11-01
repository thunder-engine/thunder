#ifndef STRINGPROPERTY_H
#define STRINGPROPERTY_H

#include "Property.h"

class StringProperty : public Property {
    Q_OBJECT

public:
    explicit StringProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged();

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // STRINGPROPERTY_H
