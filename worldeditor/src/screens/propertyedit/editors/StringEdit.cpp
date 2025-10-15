#include "StringEdit.h"
#include "ui_StringEdit.h"

#include <QTimer>

StringEdit::StringEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::StringEdit) {

    ui->setupUi(this);
    ui->pushButton->hide();

    connect(ui->lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editFinished()));

    ui->lineEdit->installEventFilter(this);
}

StringEdit::~StringEdit() {
    delete ui;
}

void StringEdit::setData(const Variant &data) {
    ui->lineEdit->setText(data.toString().data());
}

Variant StringEdit::data() const {
    return TString(ui->lineEdit->text().toStdString());
}

bool StringEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
