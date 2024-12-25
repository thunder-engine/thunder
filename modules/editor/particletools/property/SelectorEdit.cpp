#include "SelectorEdit.h"
#include "ui_SelectorEdit.h"

SelectorEdit::SelectorEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::SelectorEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SIGNAL(dataChanged()));
}

SelectorEdit::~SelectorEdit() {
    delete ui;
}

QVariant SelectorEdit::data() const {
    m_current.current = ui->comboBox->currentText();
    return QVariant::fromValue(m_current);
}

void SelectorEdit::setData(const QVariant &data) {
    SelectorData selector = data.value<SelectorData>();

    if(m_current.values != selector.values) {
        m_current = selector;
        ui->comboBox->clear();
        ui->comboBox->addItems(m_current.values);
    }

    if(m_current.current != selector.current) {
        m_current.current = selector.current;
    }

    int index = ui->comboBox->findText(selector.current);

    if(index > -1) {
        ui->comboBox->blockSignals(true);
        ui->comboBox->setCurrentIndex(index);
        ui->comboBox->blockSignals(false);
    }
}
