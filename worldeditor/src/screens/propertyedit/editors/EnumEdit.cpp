#include "EnumEdit.h"
#include "ui_EnumEdit.h"

#include <QMetaProperty>

EnumEdit::EnumEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::EnumEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SIGNAL(dataChanged()));
}

EnumEdit::~EnumEdit() {
    delete ui;
}

QVariant EnumEdit::data() const {
    return ui->comboBox->currentText();
}

void EnumEdit::setData(const QVariant &data) {
    const QMetaObject *meta = m_qObject->metaObject();
    int index = meta->indexOfProperty(qPrintable(m_propertyName));
    QMetaProperty prop = meta->property(index);

    QString key = prop.enumerator().valueToKey(data.toInt());
    index = ui->comboBox->findText(key);
    if(index > -1) {
        ui->comboBox->blockSignals(true);
        ui->comboBox->setCurrentIndex(index);
        ui->comboBox->blockSignals(false);
    }
}

void EnumEdit::setObject(QObject *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    ui->comboBox->clear();

    const QMetaObject *meta = m_qObject->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(name)));

    if(prop.isEnumType()) {
        QStringList enums;
        QMetaEnum qenum = prop.enumerator();
        for(int i = 0; i < qenum.keyCount(); i++) {
            enums << qenum.key(i);
        }
        ui->comboBox->addItems(enums);
    }
}
