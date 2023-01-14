#ifndef VECTOR4PROPERTY_H
#define VECTOR4PROPERTY_H

#include <editor/property.h>

#include <amath.h>
Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

class Vector4Property : public Property {
    Q_OBJECT

public:
    explicit Vector4Property(const QString &name = QString(), QObject *propertyObject = nullptr, int components = 4, QObject *parent = nullptr);

protected slots:
    void onDataChanged(const QVariant &data);

private:
    QVariant value(int role = Qt::UserRole) const override;

    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

    int m_components;

};

#endif // VECTOR4PROPERTY_H
