#include "localeedit.h"
#include "ui_localeedit.h"

LocaleEdit::LocaleEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::LocaleEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SIGNAL(currentIndexChanged(const QString &)));
}

LocaleEdit::~LocaleEdit() {
    delete ui;
}

void LocaleEdit::addItems(const QStringList &items) {
    ui->comboBox->addItems(items);
}

void LocaleEdit::addItem(const QString &text, const QVariant &data) {
    ui->comboBox->addItem(text, data);
}

void LocaleEdit::clear() {
    ui->comboBox->clear();
}

int LocaleEdit::findText(const QString &text) {
    return ui->comboBox->findText(text);
}

int LocaleEdit::findData(const QVariant &data) {
    return ui->comboBox->findData(data);
}

void LocaleEdit::setCurrentIndex(int index) {
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}

QString LocaleEdit::currentText() const {
    return ui->comboBox->currentText();
}

QVariant LocaleEdit::currentData() const {
    return ui->comboBox->currentData();
}
