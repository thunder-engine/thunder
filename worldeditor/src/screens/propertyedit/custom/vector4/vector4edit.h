#ifndef VECTOR4EDIT_H
#define VECTOR4EDIT_H

#include <amath.h>
#include <editor/propertyedit.h>

namespace Ui {
    class Vector4Edit;
}

class Vector4Edit : public PropertyEdit {
    Q_OBJECT

public:
    explicit Vector4Edit(QWidget *parent = nullptr);
    ~Vector4Edit();

    Variant data() const override;
    void setData(const Variant &) override;

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setComponents(uint8_t value);

private:
    Ui::Vector4Edit *ui;

    int m_components;

};

#endif // VECTOR4EDIT_H
