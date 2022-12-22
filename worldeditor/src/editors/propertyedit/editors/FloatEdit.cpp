#include "FloatEdit.h"
#include "ui_FloatEdit.h"

#include <float.h>

#include <QDoubleValidator>
#include <QTimer>

#define SCALE 100

FloatEdit::FloatEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FloatEdit) {
    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &FloatEdit::editingFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &FloatEdit::onValueChanged);

    ui->lineEdit->installEventFilter(this);

    ui->horizontalSlider->setVisible(false);
}

FloatEdit::~FloatEdit() {
    delete ui;
}

void FloatEdit::setInterval(double min, double max) {
    ui->horizontalSlider->setRange(min * SCALE, max * SCALE);

    ui->horizontalSlider->setVisible(true);
}

void FloatEdit::setValue(double value) {
    ui->lineEdit->setText(QString::number(value, 'f', 4).remove(QRegExp("\\.?0+$")));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value * SCALE);
    ui->horizontalSlider->blockSignals(false);
}

double FloatEdit::value() const {
    return ui->lineEdit->text().toFloat();
}

void FloatEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number((double)value / (double)SCALE, 'f', 4));

    emit editingFinished();
}

bool FloatEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
