#include "AlignmentEdit.h"
#include "ui_AlignmentEdit.h"

AlignmentEdit::AlignmentEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AlignmentEdit) {
    ui->setupUi(this);

    ui->left->setProperty("blue", true);
    ui->center->setProperty("blue", true);
    ui->right->setProperty("blue", true);

    ui->top->setProperty("blue", true);
    ui->middle->setProperty("blue", true);
    ui->bottom->setProperty("blue", true);

    ui->left->setProperty("checkred", true);
    ui->center->setProperty("checkred", true);
    ui->right->setProperty("checkred", true);

    ui->top->setProperty("checkred", true);
    ui->middle->setProperty("checkred", true);
    ui->bottom->setProperty("checkred", true);

    connect(ui->left, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->center, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->right, &QPushButton::toggled, this, &AlignmentEdit::onToggle);

    connect(ui->top, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->middle, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
    connect(ui->bottom, &QPushButton::toggled, this, &AlignmentEdit::onToggle);
}

AlignmentEdit::~AlignmentEdit() {
    delete ui;
}

int AlignmentEdit::alignment() const {
    int value = Left | Top;
    if(ui->center->isChecked()) {
        value &= ~Left;
        value &= ~Right;
        value |= Center;
    } else if(ui->right->isChecked()) {
        value &= ~Left;
        value |= Right;
        value &= ~Center;
    }

    if(ui->middle->isChecked()) {
        value &= ~Top;
        value &= ~Bottom;
        value |= Middle;
    } else if(ui->bottom->isChecked()) {
        value &= ~Top;
        value |= Bottom;
        value &= ~Middle;
    }
    return value;
}

void AlignmentEdit::setAlignment(int value) {
    ui->center->setChecked(value & Center);
    ui->right->setChecked(value & Right);
    ui->left->setChecked(value & Left);

    ui->top->setChecked(value & Top);
    ui->bottom->setChecked(value & Bottom);
    ui->middle->setChecked(value & Middle);
}

void AlignmentEdit::onToggle() {
    if(sender() == ui->top) {
        ui->middle->blockSignals(true);
        ui->middle->setChecked(false);
        ui->middle->blockSignals(false);

        ui->bottom->blockSignals(true);
        ui->bottom->setChecked(false);
        ui->bottom->blockSignals(false);
    } else if(sender() == ui->middle) {
        ui->top->blockSignals(true);
        ui->top->setChecked(false);
        ui->top->blockSignals(false);

        ui->bottom->blockSignals(true);
        ui->bottom->setChecked(false);
        ui->bottom->blockSignals(false);
    } else if(sender() == ui->bottom) {
        ui->top->blockSignals(true);
        ui->top->setChecked(false);
        ui->top->blockSignals(false);

        ui->middle->blockSignals(true);
        ui->middle->setChecked(false);
        ui->middle->blockSignals(false);
    }

    if(sender() == ui->left) {
        ui->right->blockSignals(true);
        ui->right->setChecked(false);
        ui->right->blockSignals(false);

        ui->center->blockSignals(true);
        ui->center->setChecked(false);
        ui->center->blockSignals(false);
    } else if(sender() == ui->right) {
        ui->left->blockSignals(true);
        ui->left->setChecked(false);
        ui->left->blockSignals(false);

        ui->center->blockSignals(true);
        ui->center->setChecked(false);
        ui->center->blockSignals(false);
    } else if(sender() == ui->center) {
        ui->left->blockSignals(true);
        ui->left->setChecked(false);
        ui->left->blockSignals(false);

        ui->right->blockSignals(true);
        ui->right->setChecked(false);
        ui->right->blockSignals(false);
    }

    int value = alignment();
    emit alignmentChanged(value);
}
