#ifndef AXISESEDIT_H
#define AXISESEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class AxisesEdit;
}

class AxisesEdit : public PropertyEdit {
    Q_OBJECT

public:
    enum Axises {
        AXIS_X = (1<<0),
        AXIS_Y = (1<<1),
        AXIS_Z = (1<<2)
    };

public:
    explicit AxisesEdit(QWidget *parent = nullptr);
    ~AxisesEdit();

    QVariant data() const;
    void setData(const QVariant &value);

private:
    Ui::AxisesEdit *ui;

};

#endif // AXISESEDIT_H
