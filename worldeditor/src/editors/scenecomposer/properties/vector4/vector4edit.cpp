#include "vector4edit.h"
#include "ui_vector4edit.h"

#include <QDoubleValidator>
#include <QLocale>
#include <QTimer>

#include <float.h>

Vector4Edit::Vector4Edit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::Vector4Edit) {

    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->x->setValidator(validator);
    ui->y->setValidator(validator);
    ui->z->setValidator(validator);
    ui->w->setValidator(validator);

    connect(ui->x, &QLineEdit::editingFinished, this, &Vector4Edit::editFinished);
    connect(ui->y, &QLineEdit::editingFinished, this, &Vector4Edit::editFinished);
    connect(ui->z, &QLineEdit::editingFinished, this, &Vector4Edit::editFinished);
    connect(ui->w, &QLineEdit::editingFinished, this, &Vector4Edit::editFinished);

    ui->x->installEventFilter(this);
    ui->y->installEventFilter(this);
    ui->z->installEventFilter(this);
    ui->w->installEventFilter(this);
}

Vector4Edit::~Vector4Edit() {
    delete ui;
}

QVariant Vector4Edit::data() const {
    Vector4 v(ui->x->text().toFloat(),
              ui->y->text().toFloat(),
              ui->z->text().toFloat(),
              ui->w->text().toFloat());

    switch(m_components) {
        case 2: return QVariant::fromValue(Vector2(v.x, v.y));
        case 3: return QVariant::fromValue(Vector3(v.x, v.y, v.z));
        default: return QVariant::fromValue(v);
    }
}

void Vector4Edit::setData(const QVariant &data) {
    int32_t userType = data.userType();
    uint8_t components = 1;
    if(userType == qMetaTypeId<Vector2>()) {
        components = 2;
    } else if(userType == qMetaTypeId<Vector3>()) {
        components = 3;
    } else if(userType == qMetaTypeId<Vector4>()) {
        components = 4;
    }
    setComponents(components);

    Vector4 v;
    switch(m_components) {
        case 2: v = Vector4(data.value<Vector2>(), 0.0f, 0.0f); break;
        case 3: v = Vector4(data.value<Vector3>(), 0.0f); break;
        default: v = data.value<Vector4>(); break;
    }

    QRegExp reg("\\.?0+$");
    ui->x->setText(QString::number(v.x, 'f', 4).remove(reg));
    ui->y->setText(QString::number(v.y, 'f', 4).remove(reg));
    ui->z->setText(QString::number(v.z, 'f', 4).remove(reg));
    ui->w->setText(QString::number(v.w, 'f', 4).remove(reg));
}

void Vector4Edit::setComponents(uint8_t value) {
    if(m_components != value) {
        m_components = value;
        ui->z->show();
        ui->w->show();

        if(value <= 3) {
            ui->w->hide();
        }

        if(value <= 2) {
            ui->z->hide();
        }
    }
}

bool Vector4Edit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
