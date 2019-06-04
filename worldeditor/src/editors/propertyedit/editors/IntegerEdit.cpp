#include "IntegerEdit.h"
#include "ui_IntegerEdit.h"

IntegerEdit::IntegerEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::IntegerEdit) {
    ui->setupUi(this);

    ui->spinBox->setProperty("minimum", -INT_MAX);
    ui->spinBox->setProperty("maximum",  INT_MAX);
}

IntegerEdit::~IntegerEdit() {
    delete ui;
}

void IntegerEdit::setValue(int32_t value) {
    ui->spinBox->setValue(value);
}

int32_t IntegerEdit::value() const {
    return ui->spinBox->value();
}
