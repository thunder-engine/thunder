#ifndef VECTOR4EDIT_H
#define VECTOR4EDIT_H

#include <amath.h>
#include <editor/propertyedit.h>

namespace Ui {
    class Vector4Edit;
}

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

class Vector4Edit : public PropertyEdit {
    Q_OBJECT

public:
    explicit Vector4Edit(QWidget *parent = nullptr);
    ~Vector4Edit();

    QVariant data() const;
    void setData(const QVariant &);

    void setComponents(uint8_t value);

private:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::Vector4Edit *ui;

    int m_components;

};

#endif // VECTOR4EDIT_H
