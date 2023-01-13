#ifndef ACTIONPROPERTY_H
#define ACTIONPROPERTY_H

#include <editor/property.h>

class ActionProperty : public Property {
    Q_OBJECT

public:
    explicit ActionProperty(const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0, bool root = false);

    bool isReadOnly() const override;

    bool isChecked() const override;
    void setChecked(bool value) override;

private:
    QWidget *createEditor(QWidget *parent) const override;

};

#endif // ACTIONPROPERTY_H
