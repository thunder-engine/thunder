#include "BooleanEdit.h"

#include "ui_BooleanEdit.h"

BooleanEdit::BooleanEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::BooleanEdit) {
    ui->setupUi(this);

    connect(ui->checkBox, &QCheckBox::stateChanged, this, &BooleanEdit::dataChanged);
}

BooleanEdit::~BooleanEdit() {
    delete ui;
}

Variant BooleanEdit::data() const {
    if(ui->checkBox->checkState() == Qt::PartiallyChecked) {
        return Variant();
    }
    return ui->checkBox->isChecked();
}

void BooleanEdit::setData(const Variant &data) {
    ui->checkBox->setTristate(false);
    ui->checkBox->setChecked(data.toBool());
}
