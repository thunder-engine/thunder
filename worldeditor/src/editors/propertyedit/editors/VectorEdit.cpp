#include "VectorEdit.h"
#include "ui_VectorEdit.h"

#include <QDoubleValidator>
#include <QLocale>

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

    connect(ui->x, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
    connect(ui->y, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
    connect(ui->z, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
}

VectorEdit::~VectorEdit() {
    delete ui;
}

Vector3 VectorEdit::data() const {
    QLocale locale;
    return Vector3(locale.toFloat(ui->x->text()),
                   locale.toFloat(ui->y->text()),
                   locale.toFloat(ui->z->text()));
}

void VectorEdit::setData(const Vector3 &v) {
    ui->x->blockSignals(true);
    ui->x->setText(QString::number(v.x, 'f', 3));
    ui->x->blockSignals(false);

    ui->y->blockSignals(true);
    ui->y->setText(QString::number(v.y, 'f', 3));
    ui->y->blockSignals(false);

    ui->z->blockSignals(true);
    ui->z->setText(QString::number(v.z, 'f', 3));
    ui->z->blockSignals(false);
}

void VectorEdit::onValueChanged() {
    emit dataChanged(QVariant::fromValue(data()));
}
