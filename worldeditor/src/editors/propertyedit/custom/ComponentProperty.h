#ifndef COMPONENTPROPERTY_H
#define COMPONENTPROPERTY_H

#include <editor/property.h>

class ComponentProperty : public Property {
    Q_OBJECT

public:
    explicit ComponentProperty(const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0, bool root = false);

    bool isReadOnly() const override;

    bool isChecked() const override;
    void setChecked(bool value) override;

private:
    QWidget *createEditor(QWidget *parent) const override;

private:
    Object *m_object;

};

#endif // COMPONENTPROPERTY_H
