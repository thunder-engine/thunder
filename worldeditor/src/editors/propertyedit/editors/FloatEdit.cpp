#include "FloatEdit.h"
#include "ui_FloatEdit.h"

#include <float.h>

FloatEdit::FloatEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FloatEdit) {
    ui->setupUi(this);

    ui->doubleSpinBox->setProperty("minimum", -DBL_MAX);
    ui->doubleSpinBox->setProperty("maximum",  DBL_MAX);

    connect(ui->doubleSpinBox, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

FloatEdit::~FloatEdit() {
    delete ui;
}

void FloatEdit::setValue(double value) {
    ui->doubleSpinBox->setValue(value);
}

double FloatEdit::value() const {
    return ui->doubleSpinBox->value();
}
