#include "arrayedit.h"
#include "ui_arrayedit.h"

#include <QIntValidator>

#include "arrayelement.h"
#include "../../property.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ArrayEdit),
        m_height(0),
        metaType(0),
        m_dynamic(false) {
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

void ArrayEdit::setObject(Object *object, const TString &name) {
    PropertyEdit::setObject(object, name);

    const MetaObject *meta = m_object->metaObject();
    int index = meta->indexOfProperty(m_propertyName.data());
    if(index == -1) {
        for(auto it : m_object->dynamicPropertyNames()) {
            if(it == m_propertyName) {
                m_dynamic = true;
            }
        }
    } else {
        MetaProperty property = meta->property(index);
        m_typeName = TString(property.type().name());

        bool isArray = false;
        Property::trimmType(m_typeName, isArray);
        metaType = MetaType::type(m_typeName.data());
        if(metaType != 0) {
            metaType++;
        }
    }
}

void ArrayEdit::addOne() {
    if(m_list.isEmpty()) {
        if(m_object) {
            void *ptr = nullptr;
            Variant value(metaType, &ptr);

            QVariant qValue(Property::qVariant(value, TString(), m_typeName, m_object));
            m_list.push_back(qValue);
            VariantList list = { value };
            m_object->setProperty(m_propertyName.data(), list);
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
