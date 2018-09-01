#include "VectorEdit.h"
#include "ui_VectorEdit.h"

#include <QDoubleValidator>

#include <float.h>

Q_DECLARE_METATYPE(Vector3)

VectorEdit::VectorEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::VectorEdit) {

    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);

    ui->x->setValidator(validator);
    ui->y->setValidator(validator);
    ui->z->setValidator(validator);

    connect(ui->x, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
    connect(ui->y, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
    connect(ui->z, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
}

VectorEdit::~VectorEdit() {
    delete ui;
}

Vector3 VectorEdit::data() const {
    return Vector3(ui->x->text().toFloat(),
                   ui->y->text().toFloat(),
                   ui->z->text().toFloat());
}

void VectorEdit::setData(const Vector3 &v) {
    ui->x->blockSignals(true);
    ui->x->setText(QString::number(v.x));
    ui->x->blockSignals(false);

    ui->y->blockSignals(true);
    ui->y->setText(QString::number(v.y));
    ui->y->blockSignals(false);

    ui->z->blockSignals(true);
    ui->z->setText(QString::number(v.z));
    ui->z->blockSignals(false);
}

void VectorEdit::onValueChanged(QString) {
    emit dataChanged(QVariant::fromValue(data()));
}
