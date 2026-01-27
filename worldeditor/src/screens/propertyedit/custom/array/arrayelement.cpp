#include "arrayelement.h"
#include "ui_arrayelement.h"

#include <editor/propertyedit.h>

#include "../../custom/objectselect/objectselect.h"

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

Variant ArrayElement::data() const {
    if(m_editor) {
        return m_editor->data();
    }
    return Variant();
}

void ArrayElement::setData(int index, const Variant &data, const TString &editor, Object *object, const TString &typeName) {
    m_index = index;
    if(m_editor && m_editor->data().userType() == data.userType()) {
        m_editor->setData(data);
    } else {
        delete m_editor;

        m_editor = PropertyEdit::constructEditor(data.userType(), this, editor);
        if(m_editor) {
            connect(m_editor, &PropertyEdit::dataChanged, this, &ArrayElement::dataChanged);
            connect(m_editor, &PropertyEdit::editFinished, this, &ArrayElement::editFinished);
            connect(ui->toolButton, &QToolButton::clicked, this, &ArrayElement::deleteElement);

            ObjectSelect *edit = dynamic_cast<ObjectSelect *>(m_editor);
            if(edit) {
                edit->setType(typeName);
            }

            m_editor->setObject(object, TString());
            m_editor->setData(data);

            QBoxLayout *l = dynamic_cast<QBoxLayout *>(layout());
            if(l) {
                l->insertWidget(l->indexOf(ui->drag) + 1, m_editor);
            }

            resize(width(), m_editor->height());
        }
    }
}

int32_t ArrayElement::index() const {
    return m_index;
}
