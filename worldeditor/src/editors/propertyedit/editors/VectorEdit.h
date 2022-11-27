#ifndef VECTOREDIT_H
#define VECTOREDIT_H

#include <QWidget>

#include <amath.h>

namespace Ui {
    class VectorEdit;
}

class VectorEdit : public QWidget {
    Q_OBJECT

public:
    explicit VectorEdit(QWidget *parent = nullptr);
    ~VectorEdit();

    Vector4 data() const;

    void setData(const Vector4 &);

    void setComponents(uint8_t value);

signals:
    void dataChanged(const QVariant &);

protected slots:
    void onValueChanged();

private:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::VectorEdit *ui;
};

#endif // VECTOREDIT_H
