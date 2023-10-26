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

QVariant BooleanEdit::data() const {
    return ui->checkBox->isChecked();
}

void BooleanEdit::setData(const QVariant &data) {
    ui->checkBox->setChecked(data.toBool());
}
