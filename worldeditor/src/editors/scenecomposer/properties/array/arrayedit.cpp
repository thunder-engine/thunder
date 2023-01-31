#include "arrayedit.h"
#include "ui_arrayedit.h"

#include <QIntValidator>
#include <QMetaProperty>
#include <QLabel>

#include "arrayelement.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ArrayEdit),
        m_propertyObject(nullptr),
        m_dynamic(false) {
    ui->setupUi(this);

    ui->lineEdit->setValidator(new QIntValidator(0, INT32_MAX, this));
    connect(ui->addButton, &QToolButton::clicked, this, &ArrayEdit::onAddItem);
    connect(ui->removeButton, &QToolButton::clicked, this, &ArrayEdit::onRemoveItem);

    ui->addButton->setProperty("compact", true);
    ui->removeButton->setProperty("compact", true);
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

    // Update editor
    for(auto it : qAsConst(m_editors)) {
        delete it;
    }
    m_editors.clear();

    int height = 22;
    int i = 0;
    for(auto &it : m_list) {
        ArrayElement *element = new ArrayElement(this);
        element->setData(i, it, m_propertyName, m_propertyObject);

        connect(element, &ArrayElement::dataChanged, this, &ArrayEdit::onDataChanged, Qt::QueuedConnection);
        connect(element, &ArrayElement::editFinished, this, &ArrayEdit::onEditFinished, Qt::QueuedConnection);
        connect(element, &ArrayElement::deleteElement, this, &ArrayEdit::onDeleteElement, Qt::QueuedConnection);

        m_editors.push_back(element);
        layout()->addWidget(element);
        height += layout()->spacing() + element->height();
        i++;
    }
    resize(width(), height);
}

void ArrayEdit::setObject(QObject *object, const QString &name) {
    m_propertyObject = object;
    m_propertyName = name;
    const QMetaObject *meta = m_propertyObject->metaObject();
    int index = meta->indexOfProperty(qPrintable(m_propertyName));
    if(index > -1) {
        QMetaProperty property = meta->property(index);
    } else {
        index = m_propertyObject->dynamicPropertyNames().indexOf(qPrintable(m_propertyName));
        if(index > -1) {
            m_dynamic = true;
        }
    }
}

void ArrayEdit::onAddItem() {
    if(m_list.isEmpty()) {
        if(m_dynamic) {
            m_list.push_back(QVariant());
            m_propertyObject->setProperty(qPrintable(m_propertyName), m_list);
        } else { // Request object to reset property (add one element)
            const QMetaObject *meta = m_propertyObject->metaObject();
            int index = meta->indexOfProperty(qPrintable(m_propertyName));
            if(index > -1) {
                QMetaProperty property = meta->property(index);
                property.reset(m_propertyObject);
            }
        }
        m_list = m_propertyObject->property(qPrintable(m_propertyName)).toList();
    } else {
        m_list.push_back(m_list.back());
    }
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
