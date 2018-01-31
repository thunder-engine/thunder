#include "VectorEdit.h"

#include <QHBoxLayout>

#include <amath.h>
#include <float.h>

Q_DECLARE_METATYPE(Vector3)

VectorEdit::VectorEdit(QWidget *parent) :
        QWidget(parent) {

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(1);

    m_pSpinX    = new QDoubleSpinBox(this);
    m_pSpinY    = new QDoubleSpinBox(this);
    m_pSpinZ    = new QDoubleSpinBox(this);
    connect(m_pSpinX, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(m_pSpinY, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(m_pSpinZ, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));

    m_pSpinX->setRange(-DBL_MAX, DBL_MAX);
    m_pSpinY->setRange(-DBL_MAX, DBL_MAX);
    m_pSpinZ->setRange(-DBL_MAX, DBL_MAX);

    layout->addWidget(m_pSpinX);
    layout->addWidget(m_pSpinY);
    layout->addWidget(m_pSpinZ);

    setLayout(layout);
}

Vector3 VectorEdit::data() const {
    return Vector3(m_pSpinX->value(), m_pSpinY->value(), m_pSpinZ->value());
}

void VectorEdit::setData(const Vector3 &v) {
    m_pSpinX->blockSignals(true);
    m_pSpinX->setValue(v.x);
    m_pSpinX->blockSignals(false);

    m_pSpinY->blockSignals(true);
    m_pSpinY->setValue(v.y);
    m_pSpinY->blockSignals(false);

    m_pSpinZ->blockSignals(true);
    m_pSpinZ->setValue(v.z);
    m_pSpinZ->blockSignals(false);
}

void VectorEdit::onValueChanged(double) {
    emit dataChanged(QVariant::fromValue(data()));
}
