#include "axisesedit.h"
#include "ui_axisesedit.h"

AxisesEdit::AxisesEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::AxisesEdit) {
    ui->setupUi(this);

    ui->x->setProperty("checkred", true);
    ui->y->setProperty("checkgreen", true);
    ui->z->setProperty("checkblue", true);

    connect(ui->x, &QPushButton::toggled, this, &AxisesEdit::editFinished);
    connect(ui->y, &QPushButton::toggled, this, &AxisesEdit::editFinished);
    connect(ui->z, &QPushButton::toggled, this, &AxisesEdit::editFinished);
}

AxisesEdit::~AxisesEdit() {
    delete ui;
}

QVariant AxisesEdit::data() const {
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

void AxisesEdit::setData(const QVariant &data) {
    int value = data.toInt();
    ui->x->setChecked(value & AXIS_X);
    ui->y->setChecked(value & AXIS_Y);
    ui->z->setChecked(value & AXIS_Z);
}
