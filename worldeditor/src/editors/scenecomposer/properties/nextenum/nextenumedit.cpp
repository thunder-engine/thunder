#include "nextenumedit.h"
#include "ui_nextenumedit.h"

NextEnumEdit::NextEnumEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NextEnumEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SIGNAL(currentIndexChanged(const QString &)));
}

NextEnumEdit::~NextEnumEdit() {
    delete ui;
}

void NextEnumEdit::addItems(const QStringList &items) {
    ui->comboBox->addItems(items);
}

void NextEnumEdit::addItem(const QString &text, const QVariant &data) {
    ui->comboBox->addItem(text, data);
}

void NextEnumEdit::clear() {
    ui->comboBox->clear();
}

int NextEnumEdit::findText(const QString &text) {
    return ui->comboBox->findText(text);
}

int NextEnumEdit::findData(const QVariant &data) {
    return ui->comboBox->findData(data);
}

void NextEnumEdit::setCurrentIndex(int index) {
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}

QString NextEnumEdit::currentText() const {
    return ui->comboBox->currentText();
}

QVariant NextEnumEdit::currentData() const {
    return ui->comboBox->currentData();
}
