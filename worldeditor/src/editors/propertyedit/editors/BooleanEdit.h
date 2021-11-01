#ifndef BOOLEANEDIT_H
#define BOOLEANEDIT_H

#include <QWidget>

namespace Ui {
    class BooleanEdit;
}

class BooleanEdit : public QWidget {
    Q_OBJECT

public:
    explicit BooleanEdit(QWidget *parent = nullptr);
    ~BooleanEdit();

    bool value() const;
    void setValue(bool value);

signals:
    void stateChanged(int value);

private:
    Ui::BooleanEdit *ui;

};

#endif // BOOLEANEDIT_H
