#include "arrayelement.h"
#include "ui_arrayelement.h"

#include <editor/propertyedit.h>

ArrayElement::ArrayElement(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ArrayElement),
        m_editor(nullptr),
        m_index(0) {

    ui->setupUi(this);
    ui->drag->hide();
    ui->toolButton->setProperty("compact", true);
}

ArrayElement::~ArrayElement() {
    delete ui;
}

QVariant ArrayElement::data() const {
    if(m_editor) {
        return m_editor->data();
    }
    return QVariant();
}

void ArrayElement::setData(int index, const QVariant &data, const QString &name, QObject *object) {
    m_index = index;
    ui->label->setText(QString("Element %1").arg(m_index));

    m_editor = PropertyEdit::constructEditor(data.userType(), this, name, object);
    if(m_editor) {
        connect(m_editor, &PropertyEdit::dataChanged, this, &ArrayElement::dataChanged);
        connect(m_editor, &PropertyEdit::editFinished, this, &ArrayElement::editFinished);
        connect(ui->toolButton, &QToolButton::clicked, this, &ArrayElement::deleteElement);

        m_editor->setData(data);
        QBoxLayout *l = dynamic_cast<QBoxLayout *>(layout());
        if(l) {
            l->insertWidget(l->indexOf(ui->label) + 1, m_editor);
        }

        resize(width(), m_editor->height());
    }
}

int32_t ArrayElement::index() const {
    return m_index;
}
