#include "IntegerEdit.h"
#include "ui_IntegerEdit.h"

#include <limits.h>

#include <QIntValidator>
#include <QTimer>

IntegerEdit::IntegerEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::IntegerEdit) {
    ui->setupUi(this);

    QIntValidator *validator = new QIntValidator(-INT32_MAX, INT32_MAX, this);

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &IntegerEdit::editingFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &IntegerEdit::onValueChanged);

    ui->lineEdit->installEventFilter(this);

    ui->horizontalSlider->setVisible(false);
}

IntegerEdit::~IntegerEdit() {
    delete ui;
}

void IntegerEdit::setInterval(int min, int max) {
    ui->horizontalSlider->setRange(min, max);

    ui->horizontalSlider->setVisible(true);
}

void IntegerEdit::setValue(int32_t value) {
    ui->lineEdit->setText(QString::number(value));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value);
    ui->horizontalSlider->blockSignals(false);
}

int32_t IntegerEdit::value() const {
    return ui->lineEdit->text().toInt();
}

void IntegerEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number(value));

    emit editingFinished();
}

bool IntegerEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
