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

int ComboEdit::findText(const QString &text) {
    return ui->comboBox->findText(text);
}

void ComboEdit::setCurrentIndex(int index) {
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}

QString ComboEdit::currentText() const {
    return ui->comboBox->currentText();
}
