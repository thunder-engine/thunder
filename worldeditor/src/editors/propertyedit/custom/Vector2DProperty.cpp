#include "Vector2DProperty.h"
#include "Vector4DProperty.h"

#include "../editors/VectorEdit.h"
#include "../nextobject.h"

Vector2DProperty::Vector2DProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant Vector2DProperty::value(int role) const {
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

QWidget *Vector2DProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    VectorEdit *e = new VectorEdit(parent);
    e->setComponents(2);
    m_editor = e;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_editor, SIGNAL(dataChanged(QVariant)), this, SLOT(onDataChanged(QVariant)));
    return m_editor;
}

bool Vector2DProperty::setEditorData(QWidget *editor, const QVariant &data) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(Vector4(data.value<Vector2>(), 0.0, 0.0));
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant Vector2DProperty::editorData(QWidget *editor) {
    VectorEdit *e = static_cast<VectorEdit *>(editor);
    if(e) {
        Vector4 v = e->data();
        return QVariant::fromValue(Vector2(v.x, v.y));
    }
    return Property::editorData(editor);
}

QSize Vector2DProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void Vector2DProperty::onDataChanged(const QVariant &data) {
    if(data.isValid()) {
        Vector4 v = data.value<Vector4>();
        setValue(QVariant::fromValue(Vector2(v.x, v.y)));
    }
}
