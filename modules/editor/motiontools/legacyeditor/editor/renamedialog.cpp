#include "renamedialog.h"
#include "ui_renamedialog.h"

RenameDialog::RenameDialog(QWidget *parent):
        QDialog(parent),
        ui(new Ui::RenameDialog) {
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

RenameDialog::~RenameDialog() {
    delete ui;
}

QString RenameDialog::text() const {
    return ui->lineEdit->text();
}

void RenameDialog::setText(const QString &text) {
    ui->lineEdit->setText(text);
}

void RenameDialog::on_okButton_clicked() {
    accept();
}

