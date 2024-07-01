#include "FloatEdit.h"
#include "ui_FloatEdit.h"

#include <float.h>

#include <QDoubleValidator>
#include <QTimer>

#define SCALE 100

FloatEdit::FloatEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::FloatEdit) {
    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &FloatEdit::editFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &FloatEdit::onValueChanged);

    ui->lineEdit->installEventFilter(this);

    ui->horizontalSlider->setVisible(false);
}

FloatEdit::~FloatEdit() {
    delete ui;
}

void FloatEdit::setEditorHint(const QString &hint) {
    if(!hint.isEmpty()) {
        static QRegularExpression regExp {"\\d+\\.\\d+"};

        QStringList list;

        for(const QRegularExpressionMatch &match : regExp.globalMatch(hint)) {
            list << match.captured(0).trimmed();
        }

        if(list.size() == 2) {
            ui->horizontalSlider->setRange(list[0].toFloat() * SCALE, list[1].toFloat() * SCALE);

            ui->horizontalSlider->setVisible(true);
        }
    }
}

void FloatEdit::setData(const QVariant &value) {
    ui->lineEdit->setText(QString::number(value.toFloat(), 'f', 4).remove(QRegularExpression("\\.?0+$")));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value.toFloat() * SCALE);
    ui->horizontalSlider->blockSignals(false);
}

QVariant FloatEdit::data() const {
    return ui->lineEdit->text().toFloat();
}

void FloatEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number((double)value / (double)SCALE, 'f', 4));

    emit editFinished();
}

bool FloatEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
