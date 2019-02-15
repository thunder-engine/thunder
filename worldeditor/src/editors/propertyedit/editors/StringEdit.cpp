#include "StringEdit.h"
#include "ui_StringEdit.h"

StringEdit::StringEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::StringEdit) {

    ui->setupUi(this);
    ui->pushButton->hide();
    connect(ui->lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editFinished()));
}

StringEdit::~StringEdit() {
    delete ui;
}

void StringEdit::setText(const QString &text) {
    ui->lineEdit->setText(text);
}

QString StringEdit::text() const {
    return ui->lineEdit->text();
}
