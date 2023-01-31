#include "nextenumedit.h"
#include "ui_nextenumedit.h"

#include <metaobject.h>
#include <metaproperty.h>

NextEnumEdit::NextEnumEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::NextEnumEdit),
        m_metaEnum(MetaEnum(nullptr)) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString&)), this, SIGNAL(currentIndexChanged(QString&)));
}

NextEnumEdit::~NextEnumEdit() {
    delete ui;
}

QVariant NextEnumEdit::data() const {
    return QVariant::fromValue(m_value);
}

void NextEnumEdit::setData(const QVariant &data) {
    Enum value = data.value<Enum>();
    if(value.m_object) {
        m_value = value;
        const MetaObject *meta = m_value.m_object->metaObject();
        int index = meta->indexOfEnumerator(qPrintable(m_value.m_enumName));
        if(index > -1) {
            m_metaEnum = meta->enumerator(index);
            int idx = 0;
            ui->comboBox->clear();
            for(int i = 0; i < m_metaEnum.keyCount(); i++) {
                ui->comboBox->addItem(m_metaEnum.key(i));
                if(m_metaEnum.value(i) == m_value.m_value) {
                    idx = i;
                }
            }

            ui->comboBox->blockSignals(true);
            ui->comboBox->setCurrentIndex(idx);
            ui->comboBox->blockSignals(false);
        }
    }
}

void NextEnumEdit::onValueChanged(const QString &item) {
    int idx = m_value.m_value;
    for(int i = 0; i < m_metaEnum.keyCount(); i++) {
        if(item == m_metaEnum.key(i)) {
            idx = i;
            break;
        }
    }
    m_value.m_value = m_metaEnum.value(idx);

    emit editFinished();
}
