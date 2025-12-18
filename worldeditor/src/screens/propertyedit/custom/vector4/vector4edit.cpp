#include "vector4edit.h"
#include "ui_vector4edit.h"

#include <QDoubleValidator>
#include <QLocale>
#include <QTimer>

#include <float.h>

Vector4Edit::Vector4Edit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::Vector4Edit),
        m_type(0) {

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

Variant Vector4Edit::data() const {
    switch(m_type) {
        case MetaType::VECTOR2:  return Vector2(ui->x->text().toFloat(),
                                                ui->y->text().toFloat());
        case MetaType::VECTOR3:  return Vector3(ui->x->text().toFloat(),
                                                ui->y->text().toFloat(),
                                                ui->z->text().toFloat());
        case MetaType::VECTOR4:  return Vector4(ui->x->text().toFloat(),
                                                ui->y->text().toFloat(),
                                                ui->z->text().toFloat(),
                                                ui->w->text().toFloat());
        default: break;
    }
    return Variant();
}

void Vector4Edit::setData(const Variant &data) {
    static const QRegularExpression reg("\\.?0+$");

    setComponents(data.userType());

    Vector4 v;
    switch(m_type) {
        case MetaType::VECTOR2: {
            Vector2 v = data.value<Vector2>();
            ui->x->setText(QString::number(v.x, 'f', 4).remove(reg));
            ui->y->setText(QString::number(v.y, 'f', 4).remove(reg));
        } break;
        case MetaType::VECTOR3: {
            Vector3 v = data.value<Vector3>();
            ui->x->setText(QString::number(v.x, 'f', 4).remove(reg));
            ui->y->setText(QString::number(v.y, 'f', 4).remove(reg));
            ui->z->setText(QString::number(v.z, 'f', 4).remove(reg));
        } break;
        case MetaType::VECTOR4: {
            v = data.value<Vector4>();
            ui->x->setText(QString::number(v.x, 'f', 4).remove(reg));
            ui->y->setText(QString::number(v.y, 'f', 4).remove(reg));
            ui->z->setText(QString::number(v.z, 'f', 4).remove(reg));
            ui->w->setText(QString::number(v.w, 'f', 4).remove(reg));
        } break;
        default: break;
    }
}

void Vector4Edit::setComponents(uint32_t type) {
    if(m_type != type) {
        m_type = type;

        switch(m_type) {
            case MetaType::VECTOR2: {
                ui->z->hide();
                ui->w->hide();
            } break;
            case MetaType::VECTOR3: {
                ui->z->show();
                ui->w->hide();
            } break;
            default: {
                ui->z->show();
                ui->w->show();
            } break;
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
