#include "AlignmentEdit.h"
#include "ui_AlignmentEdit.h"

AlignmentEdit::AlignmentEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AlignmentEdit) {
    ui->setupUi(this);

    ui->left->setProperty("blue", true);
    ui->center->setProperty("blue", true);
    ui->right->setProperty("blue", true);

    ui->left->setProperty("checkred", true);
    ui->center->setProperty("checkred", true);
    ui->right->setProperty("checkred", true);

    connect(ui->left, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->center, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->right, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
}

AlignmentEdit::~AlignmentEdit() {
    delete ui;
}

AlignmentEdit::Alignment AlignmentEdit::alignment() const {
    Alignment value = Left;
    if(ui->center->isChecked()) {
        value = Center;
    }
    if(ui->right->isChecked()) {
        value = Right;
    }
    return value;
}

void AlignmentEdit::setAlignment(Alignment value) {
    switch (value) {
        case Center: {
            ui->center->setChecked(true);
        } break;
        case Right: {
            ui->right->setChecked(true);
        } break;
        default: {
            ui->left->setChecked(true);
        } break;
    }
}

void AlignmentEdit::onToggle() {
    emit alignmentChanged(alignment());
}
