#ifndef AXISESEDIT_H
#define AXISESEDIT_H

#include <QWidget>

namespace Ui {
class AxisesEdit;
}

class AxisesEdit : public QWidget {
    Q_OBJECT

public:
    enum Axises {
        AXIS_X = (1<<0),
        AXIS_Y = (1<<1),
        AXIS_Z = (1<<2)
    };

public:
    explicit AxisesEdit (QWidget *parent = nullptr);
    ~AxisesEdit ();

    int axises () const;
    void setAxises (int value);

signals:
    void axisesChanged (int value);

private:
    void onToggle ();

    Ui::AxisesEdit *ui;
};

#endif // AXISESEDIT_H
