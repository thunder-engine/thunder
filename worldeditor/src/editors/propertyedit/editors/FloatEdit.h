#ifndef FLOATEDIT_H
#define FLOATEDIT_H

#include <QWidget>

namespace Ui {
    class FloatEdit;
}

class FloatEdit : public QWidget {
    Q_OBJECT

public:
    explicit FloatEdit(QWidget *parent = nullptr);
    ~FloatEdit();

    void setValue(double value);

    double value() const;

    void setInterval(double min, double max);

signals:
    void editingFinished();

private slots:
    void onValueChanged(int value);

private:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::FloatEdit *ui;
};

#endif // FLOATEDIT_H
