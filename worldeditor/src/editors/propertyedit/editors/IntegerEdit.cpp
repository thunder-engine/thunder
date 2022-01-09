#include "IntegerEdit.h"
#include "ui_IntegerEdit.h"

#include <limits.h>

#include <QIntValidator>

IntegerEdit::IntegerEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::IntegerEdit) {
    ui->setupUi(this);

    QIntValidator *validator = new QIntValidator(-INT32_MAX, INT32_MAX, this);

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &IntegerEdit::editingFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &IntegerEdit::onValueChanged);

    ui->horizontalSlider->setVisible(false);
}

IntegerEdit::~IntegerEdit() {
    delete ui;
}

void IntegerEdit::setInterval(int min, int max) {
    ui->horizontalSlider->setRange(min, max);

    ui->horizontalSlider->setVisible(true);
}

void IntegerEdit::setValue(int32_t value) {
    ui->lineEdit->setText(QString::number(value));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value);
    ui->horizontalSlider->blockSignals(false);
}

int32_t IntegerEdit::value() const {
    return ui->lineEdit->text().toInt();
}

void IntegerEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number(value));

    emit editingFinished();
}
