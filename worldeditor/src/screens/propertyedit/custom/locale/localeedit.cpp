#include "localeedit.h"
#include "ui_localeedit.h"

#include <url.h>

#include <QDirIterator>
#include <QLocale>

LocaleEdit::LocaleEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::LocaleEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(dataChanged()));

    QDirIterator it(":/Translations", QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QLocale locale(Url(it.next().toStdString()).baseName().data());
        QString name = locale.nativeLanguageName();
        ui->comboBox->addItem(name.replace(0, 1, name[0].toUpper()), locale.bcp47Name());
    }
}

LocaleEdit::~LocaleEdit() {
    delete ui;
}

Variant LocaleEdit::data() const {
    return TString (ui->comboBox->currentData().toString().toStdString());
}

void LocaleEdit::setData(const Variant &data) {
    int index = ui->comboBox->findData(data.toString().data());
    if(index == -1) {
        return;
    }
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}
