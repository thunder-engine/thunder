#include "BooleanEdit.h"

#include "ui_BooleanEdit.h"

BooleanEdit::BooleanEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::BooleanEdit) {
    ui->setupUi(this);

    connect(ui->checkBox, &QCheckBox::stateChanged, this, &BooleanEdit::stateChanged);
}

BooleanEdit::~BooleanEdit() {
    delete ui;
}

void BooleanEdit::setValue(bool value) {
    ui->checkBox->setChecked(value);
}

bool BooleanEdit::value() const {
    return ui->checkBox->isChecked();
}
