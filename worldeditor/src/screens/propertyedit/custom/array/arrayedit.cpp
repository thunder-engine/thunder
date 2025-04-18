#include "arrayedit.h"
#include "ui_arrayedit.h"

#include <QIntValidator>
#include <QMetaProperty>
#include <QLabel>

#include "arrayelement.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ArrayEdit),
        m_dynamic(false),
        m_height(0) {
    ui->setupUi(this);

    ui->lineEdit->setValidator(new QIntValidator(0, INT32_MAX, this));
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &ArrayEdit::onCountChanged);

    ui->addButton->setProperty("compact", true);
    connect(ui->addButton, &QToolButton::clicked, this, &ArrayEdit::onAddItem);

    ui->removeButton->setProperty("compact", true);
    connect(ui->removeButton, &QToolButton::clicked, this, &ArrayEdit::onRemoveItem);

    m_height = height();
}

ArrayEdit::~ArrayEdit() {
    delete ui;
}

QVariant ArrayEdit::data() const {
    return m_list;
}

void ArrayEdit::setData(const QVariant &data) {
    m_list = data.toList();
    ui->lineEdit->setText(QString::number(m_list.size()));

    int32_t deltaSize = m_list.size() - m_editors.size();
    if(deltaSize > 0) { // Need to add additional editors
        for(int i = 0; i < deltaSize; i++) {
            ArrayElement *element = new ArrayElement(this);

            connect(element, &ArrayElement::dataChanged, this, &ArrayEdit::onDataChanged, Qt::QueuedConnection);
            connect(element, &ArrayElement::editFinished, this, &ArrayEdit::onEditFinished, Qt::QueuedConnection);
            connect(element, &ArrayElement::deleteElement, this, &ArrayEdit::onDeleteElement, Qt::QueuedConnection);

            m_editors.push_back(element);
            ui->content->addWidget(element);
        }
    }

    int height = m_height;

    int i = 0;
    foreach(auto element, m_editors) {
        if(i < m_list.size()) {
            element->setVisible(true);
            element->setData(i, m_list.at(i), m_propertyName);

            height += element->sizeHint().height();
            i++;
        } else {
            element->setVisible(false);
        }
    }
    resize(width(), height);
}

void ArrayEdit::setObject(QObject *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    const QMetaObject *meta = m_qObject->metaObject();
    int index = meta->indexOfProperty(qPrintable(m_propertyName));
    if(index == -1) {
        index = m_qObject->dynamicPropertyNames().indexOf(qPrintable(m_propertyName));
        if(index > -1) {
            m_dynamic = true;
        }
    }
}

void ArrayEdit::setObject(Object *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    const MetaObject *meta = m_object->metaObject();
    int index = meta->indexOfProperty(qPrintable(m_propertyName));
    if(index == -1) {
        for(auto it : m_object->dynamicPropertyNames()) {
            if(it == qPrintable(m_propertyName)) {
                m_dynamic = true;
            }
        }
    }
}

void ArrayEdit::addOne() {
    if(m_list.isEmpty()) {
        if(m_qObject) {
            if(m_dynamic) {
                m_list.push_back(QVariant());
                m_qObject->setProperty(qPrintable(m_propertyName), m_list);
            } else { // Request object to reset property (add one element)
                const QMetaObject *meta = m_qObject->metaObject();
                int index = meta->indexOfProperty(qPrintable(m_propertyName));
                if(index > -1) {
                    QMetaProperty property = meta->property(index);
                    property.reset(m_qObject);
                }
                m_list = m_qObject->property(qPrintable(m_propertyName)).toList();
            }
        }

        if(m_object) {
            m_list.push_back(QVariant());
            m_object->setProperty(qPrintable(m_propertyName), { Variant() });
        }
    } else {
        m_list.push_back(m_list.back());
    }
}

void ArrayEdit::onAddItem() {
    addOne();

    setData(m_list);
    emit editFinished();
}

void ArrayEdit::onRemoveItem() {
    if(!m_list.isEmpty()) {
        m_list.removeLast();
    }

    setData(m_list);
    emit editFinished();
}

void ArrayEdit::onCountChanged() {
    uint32_t value = ui->lineEdit->text().toUInt();

    while(value > m_list.size()) {
        addOne();
    }

    while(value < m_list.size()) {
        m_list.removeLast();
    }

    setData(m_list);
    emit editFinished();
}

void ArrayEdit::onDataChanged() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        m_list[element->index()] = element->data();
    }
    emit dataChanged();
}

void ArrayEdit::onEditFinished() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        m_list.replace(element->index(), element->data());
    }
    emit editFinished();
}

void ArrayEdit::onDeleteElement() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        m_list.removeAt(element->index());
    }
    emit editFinished();
}
