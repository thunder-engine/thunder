#ifndef VECTOREDIT_H
#define VECTOREDIT_H

#include <QWidget>
#include <QSpinBox>

#include <amath.h>

class VectorEdit : public QWidget {
    Q_OBJECT
public:
    explicit VectorEdit(QWidget *parent = 0);

    Vector3       data            () const;
    void            setData         (const Vector3 &v);

signals:
    void            dataChanged     (QVariant data);

protected slots:
    void            onValueChanged  (double);

protected:
    QDoubleSpinBox *m_pSpinX;
    QDoubleSpinBox *m_pSpinY;
    QDoubleSpinBox *m_pSpinZ;
};

#endif // VECTOREDIT_H
