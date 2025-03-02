#include "actions.h"
#include "ui_actions.h"

#include "../../propertyeditor.h"

Actions::Actions(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::Actions),
        m_property(MetaProperty(nullptr)) {

    ui->setupUi(this);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setObject(Object *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    if(m_object == nullptr) {
        return;
    }
    const MetaObject *meta = m_object->metaObject();

    int index = meta->indexOfProperty("enabled");
    if(index > -1) {
        m_property = meta->property(index);
    }

    PropertyEditor *editor = findEditor(parentWidget());
    if(editor) {
        for(auto it : editor->getActions(m_object, this)) {
            ui->horizontalLayout->addWidget(it);
        }
    }
}

void Actions::setObject(QObject *object, const QString &name) {
    PropertyEdit::setObject(object, name);

    if(m_qObject == nullptr) {
        return;
    }
    const QMetaObject *meta = m_qObject->metaObject();

    int index = meta->indexOfProperty("enabled");
    if(index > -1) {
        m_qProperty = meta->property(index);
    }

    PropertyEditor *editor = findEditor(parentWidget());
    if(editor) {
        for(auto it : editor->getActions(m_qObject, this)) {
            ui->horizontalLayout->addWidget(it);
        }
    }
}

void Actions::onDataChanged(bool value) {
    if(m_property.isValid()) {
        m_property.write(m_object, value);
    } else if(m_qProperty.isValid()) {
        m_qProperty.write(m_qObject, value);
    }
}

bool Actions::isChecked() const {
    if(m_property.isValid()) {
        return m_property.read(m_object).toBool();
    } else if(m_qProperty.isValid()) {
        return m_qProperty.read(m_qObject).toBool();
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
