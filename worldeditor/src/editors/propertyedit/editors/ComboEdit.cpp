#include "ComboEdit.h"
#include "ui_ComboEdit.h"

ComboEdit::ComboEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComboEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SIGNAL(currentIndexChanged(const QString &)));
}

ComboEdit::~ComboEdit() {
    delete ui;
}

void ComboEdit::addItems(const QStringList &items) {
    ui->comboBox->addItems(items);
}

void ComboEdit::addItem(const QString &text, const QVariant &data) {
    ui->comboBox->addItem(text, data);
}

void ComboEdit::clear() {
    ui->comboBox->clear();
}

int ComboEdit::findText(const QString &text) {
    return ui->comboBox->findText(text);
}

int ComboEdit::findData(const QVariant &data) {
    return ui->comboBox->findData(data);
}

void ComboEdit::setCurrentIndex(int index) {
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}

QString ComboEdit::currentText() const {
    return ui->comboBox->currentText();
}

QVariant ComboEdit::currentData() const {
    return ui->comboBox->currentData();
}
