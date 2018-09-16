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

    connect(ui->x, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
    connect(ui->y, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
    connect(ui->z, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged(QString)));
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
    QLocale locale;
    ui->x->blockSignals(true);
    ui->x->setText(locale.toString(v.x));
    ui->x->blockSignals(false);

    ui->y->blockSignals(true);
    ui->y->setText(locale.toString(v.y));
    ui->y->blockSignals(false);

    ui->z->blockSignals(true);
    ui->z->setText(locale.toString(v.z));
    ui->z->blockSignals(false);
}

void VectorEdit::onValueChanged(QString) {
    emit dataChanged(QVariant::fromValue(data()));
}
