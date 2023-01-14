#include "axisesedit.h"
#include "ui_axisesedit.h"

AxisesEdit::AxisesEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AxisesEdit) {
    ui->setupUi(this);

    ui->x->setProperty("checkred", true);
    ui->y->setProperty("checkgreen", true);
    ui->z->setProperty("checkblue", true);

    connect(ui->x, &QPushButton::toggled, this, &AxisesEdit::onToggle);
    connect(ui->y, &QPushButton::toggled, this, &AxisesEdit::onToggle);
    connect(ui->z, &QPushButton::toggled, this, &AxisesEdit::onToggle);
}

AxisesEdit::~AxisesEdit() {
    delete ui;
}

int AxisesEdit::axises() const {
    int value = 0;
    if(ui->x->isChecked()) {
        value |= AXIS_X;
    }
    if(ui->y->isChecked()) {
        value |= AXIS_Y;
    }
    if(ui->z->isChecked()) {
        value |= AXIS_Z;
    }
    return value;
}

void AxisesEdit::setAxises(int value) {
    ui->x->setChecked(value & AXIS_X);
    ui->y->setChecked(value & AXIS_Y);
    ui->z->setChecked(value & AXIS_Z);
}

void AxisesEdit::onToggle() {
    emit axisesChanged(axises());
}
