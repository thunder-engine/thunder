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
    m_current.current = ui->comboBox->currentText().toStdString();
    return QVariant::fromValue(m_current);
}

void SelectorEdit::setData(const QVariant &data) {
    SelectorData selector = data.value<SelectorData>();

    if(m_current.values != selector.values) {
        m_current = selector;
        ui->comboBox->clear();

        QStringList values;
        for(auto it : m_current.values) {
            values << it.c_str();
        }

        ui->comboBox->addItems(values);
    }

    if(m_current.current != selector.current) {
        m_current.current = selector.current;
    }

    int index = ui->comboBox->findText(selector.current.c_str());

    if(index > -1) {
        ui->comboBox->blockSignals(true);
        ui->comboBox->setCurrentIndex(index);
        ui->comboBox->blockSignals(false);
    }
}
