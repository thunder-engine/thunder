#include "nextenumedit.h"
#include "ui_nextenumedit.h"

#include <metaobject.h>
#include <metaproperty.h>

NextEnumEdit::NextEnumEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::NextEnumEdit),
        m_metaEnum(MetaEnum(nullptr)) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged(int)));
}

NextEnumEdit::~NextEnumEdit() {
    delete ui;
}

Variant NextEnumEdit::data() const {
    return m_value;
}

void NextEnumEdit::setData(const Variant &data) {
    m_value = data.toInt();

    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentText(m_metaEnum.valueToKey(m_value));
    ui->comboBox->blockSignals(false);
}

void NextEnumEdit::setEnumData(const TString &name, Object *object) {
    m_enumName = name;
    m_object = object;

    const MetaObject *meta = m_object->metaObject();
    int index = meta->indexOfEnumerator(m_enumName.data());
    if(index > -1) {
        m_metaEnum = meta->enumerator(index);

        ui->comboBox->blockSignals(true);
        ui->comboBox->clear();
        for(int i = 0; i < m_metaEnum.keyCount(); i++) {
            std::string key = m_metaEnum.key(i);
            if(key.front() != '_') {
                 ui->comboBox->addItem(key.c_str());
            }
        }

        ui->comboBox->blockSignals(false);
    }
}

void NextEnumEdit::onValueChanged(int index) {
    m_value = m_metaEnum.keyToValue(qPrintable(ui->comboBox->itemText(index)));

    emit editFinished();
}
