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

void StringEdit::setData(const QVariant &data) {
    ui->lineEdit->setText(data.toString());
}

QVariant StringEdit::data() const {
    return ui->lineEdit->text();
}

bool StringEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
