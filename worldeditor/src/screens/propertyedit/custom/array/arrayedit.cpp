#include "arrayedit.h"
#include "ui_arrayedit.h"

#include <QIntValidator>

#include "arrayelement.h"
#include "../../property.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ArrayEdit),
        m_height(0),
        m_metaType(0),
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

Variant ArrayEdit::data() const {
    return m_list;
}

void ArrayEdit::setData(const Variant &data) {
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
    for(auto element : m_editors) {
        if(i < m_list.size()) {
            element->setVisible(true);

            element->setData(i, *std::next(m_list.begin(), i), m_editorName, m_object, m_typeName);

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

    m_propertyName = name;

    const MetaObject *meta = m_object->metaObject();
    int index = meta->indexOfProperty(m_propertyName.data());
    if(index == -1) {
        for(auto &it : m_object->dynamicPropertyNames()) {
            if(it == m_propertyName) {
                m_dynamic = true;
            }
        }
    } else {
        MetaProperty property = meta->property(index);
        m_typeName = TString(property.type().name());

        bool isArray = false;
        Property::trimmType(m_typeName, isArray);

        m_metaType = MetaType::type(m_typeName.data());
        auto factory = Engine::metaFactory(m_typeName);
        if(factory) {
            m_metaType++;
        }

        TString hints;
        const char *annotation = property.table()->annotation;
        if(annotation) {
            hints = annotation;
        }

        m_editorName = Property::editorName(hints, m_typeName);
    }
}

void ArrayEdit::addOne() {
    if(m_list.empty()) {
        if(m_object) {
            Variant value;
            switch(m_metaType) {
                case MetaType::BOOLEAN: value = Variant(false); break;
                case MetaType::INTEGER: value = Variant(0); break;
                case MetaType::FLOAT: value = Variant(0.0f); break;
                case MetaType::STRING: value = Variant(TString()); break;
                case MetaType::VECTOR2: value = Variant(Vector2()); break;
                case MetaType::VECTOR3: value = Variant(Vector3()); break;
                case MetaType::VECTOR4: value = Variant(Vector4()); break;
                default: {
                    void *ptr = nullptr;
                    value = Variant(m_metaType, &ptr);
                } break;
            }

            m_list.push_back(value);
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
    if(!m_list.empty()) {
        m_list.pop_back();
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
        m_list.pop_back();
    }

    setData(m_list);
    emit editFinished();
}

void ArrayEdit::onDataChanged() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        *std::next(m_list.begin(), element->index()) = element->data();
    }
    emit dataChanged();
}

void ArrayEdit::onEditFinished() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        *std::next(m_list.begin(), element->index()) = element->data();
    }
    emit editFinished();
}

void ArrayEdit::onDeleteElement() {
    ArrayElement *element = dynamic_cast<ArrayElement *>(sender());
    if(element) {
        m_list.erase( std::next(m_list.begin(), element->index()) );
    }
    emit editFinished();
}
