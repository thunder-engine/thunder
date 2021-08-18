#include "Vector3DProperty.h"
#include "Vector4DProperty.h"

#include "editors/propertyedit/editors/VectorEdit.h"
#include "../nextobject.h"

Vector3DProperty::Vector3DProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant Vector3DProperty::value(int role) const {
    QVariant data = Property::value(role);
    if(data.isValid() && role != Qt::UserRole) {
        switch(role) {
            case Qt::DisplayRole: {
                return data;
            }
            default: break;
        }
    }
    return data;
}

QWidget *Vector3DProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    VectorEdit *e = new VectorEdit(parent);
    e->setComponents(3);
    m_editor = e;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_editor, SIGNAL(dataChanged(QVariant)), this, SLOT(onDataChanged(QVariant)));
    return m_editor;
}

bool Vector3DProperty::setEditorData(QWidget *editor, const QVariant &data) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(Vector4(data.value<Vector3>(), 0.0));
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant Vector3DProperty::editorData(QWidget *editor) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        Vector4 v = e->data();
        return QVariant::fromValue(Vector3(v.x, v.y, v.z));
    }
    return Property::editorData(editor);
}

QSize Vector3DProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void Vector3DProperty::onDataChanged(const QVariant &data) {
    if(data.isValid()) {
        Vector4 v = data.value<Vector4>();
        setValue(QVariant::fromValue(Vector3(v.x, v.y, v.z)));
    }
}
