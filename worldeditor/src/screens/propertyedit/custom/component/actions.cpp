#include "actions.h"
#include "ui_actions.h"

#include "../../nextobject.h"

#include "../../propertyeditor.h"

Actions::Actions(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::Actions),
        m_property(MetaProperty(nullptr)),
        m_object(nullptr) {

    ui->setupUi(this);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setObject(Object *object, const QString &name) {
    m_object = object;

    if(m_object == nullptr) {
        return;
    }
    const MetaObject *meta = m_object->metaObject();

    int32_t index = meta->indexOfProperty(qPrintable(m_propertyName + "/enabled"));
    if(index == -1) {
        index = meta->indexOfProperty("enabled");
    }
    if(index > -1) {
        m_property = meta->property(index);
    }
}

void Actions::setObject(QObject *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    NextObject *next = dynamic_cast<NextObject *>(m_propertyObject);
    if(next) {
        setObject(next->component(m_propertyName), m_propertyName);
    }

    PropertyEditor *editor = findEditor(parentWidget());
    if(editor) {
        for(auto it : editor->getActions(m_propertyObject, m_propertyName, this)) {
            ui->horizontalLayout->addWidget(it);
        }
    }
}

void Actions::onDataChanged(bool value) {
    if(m_property.isValid()) {
        m_property.write(m_object, value);
    }
}

bool Actions::isChecked() const {
    if(m_property.isValid()) {
        return m_property.read(m_object).toBool();
    }
    return false;
}

PropertyEditor *Actions::findEditor(QWidget *parent) const {
    PropertyEditor *editor = dynamic_cast<PropertyEditor *>(parent);
    if(editor) {
        return editor;
    }

    if(parent) {
        return findEditor(parent->parentWidget());
    }

    return nullptr;
}
