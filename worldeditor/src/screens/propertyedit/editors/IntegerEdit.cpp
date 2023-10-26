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

QVariant IntegerEdit::data() const {
    return ui->lineEdit->text().toInt();
}

void IntegerEdit::setData(const QVariant &data) {
    int32_t value = data.toInt();
    ui->lineEdit->setText(QString::number(value));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value);
    ui->horizontalSlider->blockSignals(false);
}

void IntegerEdit::setEditorHint(const QString &hint) {
    if(!hint.isEmpty()) {
        static QRegExp regExp {"\\d+"};

        QStringList list;
        int pos = 0;

        while((pos = regExp.indexIn(hint, pos)) != -1) {
            list << regExp.cap(0);
            pos += regExp.matchedLength();
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
