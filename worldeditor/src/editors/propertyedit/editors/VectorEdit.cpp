#include "VectorEdit.h"
#include "ui_VectorEdit.h"

#include <float.h>

Q_DECLARE_METATYPE(Vector3)

VectorEdit::VectorEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::VectorEdit) {

    ui->setupUi(this);

    ui->x->setRange(-DBL_MAX, DBL_MAX);
    ui->y->setRange(-DBL_MAX, DBL_MAX);
    ui->z->setRange(-DBL_MAX, DBL_MAX);

    connect(ui->x, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(ui->y, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(ui->z, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

VectorEdit::~VectorEdit() {
    delete ui;
}

Vector3 VectorEdit::data() const {
    return Vector3(ui->x->value(), ui->y->value(), ui->z->value());
}

void VectorEdit::setData(const Vector3 &v) {
    ui->x->blockSignals(true);
    ui->x->setValue(v.x);
    ui->x->blockSignals(false);

    ui->y->blockSignals(true);
    ui->y->setValue(v.y);
    ui->y->blockSignals(false);

    ui->z->blockSignals(true);
    ui->z->setValue(v.z);
    ui->z->blockSignals(false);
}

void VectorEdit::onValueChanged(double) {
    emit dataChanged(QVariant::fromValue(data()));
}
