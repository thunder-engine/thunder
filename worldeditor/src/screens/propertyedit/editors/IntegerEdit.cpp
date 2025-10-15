#include "IntegerEdit.h"
#include "ui_IntegerEdit.h"

#include <limits.h>

#include <QIntValidator>
#include <QTimer>

IntegerEdit::IntegerEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::IntegerEdit) {
    ui->setupUi(this);

    QIntValidator *validator = new QIntValidator(-INT32_MAX, INT32_MAX, this);

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &IntegerEdit::editFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &IntegerEdit::onValueChanged);

    ui->lineEdit->installEventFilter(this);

    ui->horizontalSlider->setVisible(false);
}

IntegerEdit::~IntegerEdit() {
    delete ui;
}

Variant IntegerEdit::data() const {
    return ui->lineEdit->text().toInt();
}

void IntegerEdit::setData(const Variant &data) {
    int32_t value = data.toInt();
    ui->lineEdit->setText(QString::number(value));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value);
    ui->horizontalSlider->blockSignals(false);
}

void IntegerEdit::setEditorHint(const TString &hint) {
    if(!hint.isEmpty()) {
        static QRegularExpression regExp {"\\d+"};

        QStringList list;

        auto it = regExp.globalMatch(hint.data());
        while(it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            list << match.captured(0).trimmed();
        }

        if(list.size() == 2) {
            ui->horizontalSlider->setRange(list[0].toInt(), list[1].toInt());

            ui->horizontalSlider->setVisible(true);
        }
    }
}

void IntegerEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number(value));

    emit editFinished();
}

bool IntegerEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
