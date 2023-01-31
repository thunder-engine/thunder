#include "localeedit.h"
#include "ui_localeedit.h"

#include <QDirIterator>
#include <QLocale>

LocaleEdit::LocaleEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::LocaleEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(dataChanged()));

    QDirIterator it(":/Translations", QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        QLocale locale(info.baseName());
        QString name = locale.nativeLanguageName();
        ui->comboBox->addItem(name.replace(0, 1, name[0].toUpper()), locale);
    }
}

LocaleEdit::~LocaleEdit() {
    delete ui;
}

QVariant LocaleEdit::data() const {
    return ui->comboBox->currentData();
}

void LocaleEdit::setData(const QVariant &data) {
    int index = ui->comboBox->findData(data);
    if(index == -1) {
        return;
    }
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}
