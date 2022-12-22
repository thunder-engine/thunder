#include "VectorEdit.h"
#include "ui_VectorEdit.h"

#include <QDoubleValidator>
#include <QLocale>
#include <QTimer>

#include <float.h>

Q_DECLARE_METATYPE(Vector4)

VectorEdit::VectorEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::VectorEdit) {

    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->x->setValidator(validator);
    ui->y->setValidator(validator);
    ui->z->setValidator(validator);
    ui->w->setValidator(validator);

    connect(ui->x, &QLineEdit::editingFinished, this, &VectorEdit::onValueChanged);
    connect(ui->y, &QLineEdit::editingFinished, this, &VectorEdit::onValueChanged);
    connect(ui->z, &QLineEdit::editingFinished, this, &VectorEdit::onValueChanged);
    connect(ui->w, &QLineEdit::editingFinished, this, &VectorEdit::onValueChanged);

    ui->x->installEventFilter(this);
    ui->y->installEventFilter(this);
    ui->z->installEventFilter(this);
    ui->w->installEventFilter(this);
}

VectorEdit::~VectorEdit() {
    delete ui;
}

Vector4 VectorEdit::data() const {
    return Vector4(ui->x->text().toFloat(),
                   ui->y->text().toFloat(),
                   ui->z->text().toFloat(),
                   ui->w->text().toFloat());
}

void VectorEdit::setData(const Vector4 &v) {
    QRegExp reg("\\.?0+$");
    ui->x->setText(QString::number(v.x, 'f', 4).remove(reg));
    ui->y->setText(QString::number(v.y, 'f', 4).remove(reg));
    ui->z->setText(QString::number(v.z, 'f', 4).remove(reg));
    ui->w->setText(QString::number(v.w, 'f', 4).remove(reg));
}

void VectorEdit::setComponents(uint8_t value) {
    ui->z->show();
    ui->w->show();

    if(value <= 3) {
        ui->w->hide();
    }

    if(value <= 2) {
        ui->z->hide();
    }
}

void VectorEdit::onValueChanged() {
    Vector4 value = data();
    emit dataChanged(QVariant::fromValue(value));
}

bool VectorEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
