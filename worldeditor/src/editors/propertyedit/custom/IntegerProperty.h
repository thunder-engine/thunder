#ifndef INTEGERPROPERTY_H
#define INTEGERPROPERTY_H

#include <editor/property.h>

class IntegerProperty : public Property {
    Q_OBJECT

public:
    explicit IntegerProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged();

protected:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // INTEGERPROPERTY_H
