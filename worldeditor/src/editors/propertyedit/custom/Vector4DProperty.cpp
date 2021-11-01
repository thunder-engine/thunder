#include "Vector4DProperty.h"

#include "../editors/VectorEdit.h"

Vector4DProperty::Vector4DProperty(const QString &name, QObject *propertyObject, int components, QObject *parent) :
        Property(name, propertyObject, parent),
        m_components(components) {

}

QVariant Vector4DProperty::value(int role) const {
    QVariant data = Property::value(role);
    if(data.isValid() && role != Qt::UserRole) {
        switch(role) {
            case Qt::DisplayRole: return data;
            default: break;
        }
    }
    return data;
}

QWidget *Vector4DProperty::createEditor(QWidget *parent) const {
    VectorEdit *e = new VectorEdit(parent);
    e->setComponents(m_components);
    m_editor = e;
    m_editor->setDisabled(isReadOnly());
    connect(m_editor, SIGNAL(dataChanged(QVariant)), this, SLOT(onDataChanged(QVariant)));
    return m_editor;
}

bool Vector4DProperty::setEditorData(QWidget *editor, const QVariant &data) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        switch(m_components) {
        case 2: e->setData(Vector4(data.value<Vector2>(), 0.0f, 0.0f)); break;
        case 3: e->setData(Vector4(data.value<Vector3>(), 0.0f)); break;
        default: e->setData(data.value<Vector4>()); break;
        }
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant Vector4DProperty::editorData(QWidget *editor) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        Vector4 v = e->data();
        switch(m_components) {
        case 2: return QVariant::fromValue(Vector2(v.x, v.y));
        case 3: return QVariant::fromValue(Vector3(v.x, v.y, v.z));
        default: return QVariant::fromValue(v);
        }
    }
    return Property::editorData(editor);
}

void Vector4DProperty::onDataChanged(const QVariant &data) {
    if(data.isValid()) {
        Vector4 v = data.value<Vector4>();
        switch(m_components) {
        case 2: setValue(QVariant::fromValue(Vector2(v.x, v.y))); break;
        case 3: setValue(QVariant::fromValue(Vector3(v.x, v.y, v.z))); break;
        default: setValue(QVariant::fromValue(v)); break;
        }
    }
}
