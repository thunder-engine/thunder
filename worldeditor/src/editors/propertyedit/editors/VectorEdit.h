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
    explicit VectorEdit (QWidget *parent = 0);
    ~VectorEdit         ();

    Vector3             data                () const;

    void                setData             (const Vector3 &);

signals:
    void                dataChanged         (const QVariant &);

protected slots:
    void                onValueChanged      (double);

private:
    Ui::VectorEdit     *ui;
};

#endif // VECTOREDIT_H
