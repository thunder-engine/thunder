#include "Vector4DProperty.h"

#include "editors/propertyedit/editors/VectorEdit.h"
#include "../nextobject.h"

Vector4DProperty::Vector4DProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant Vector4DProperty::value(int role) const {
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

QWidget *Vector4DProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    VectorEdit *e  = new VectorEdit(parent);
    e->setComponents(4);
    m_Editor = e;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_Editor, SIGNAL(dataChanged(QVariant)), this, SLOT(onDataChanged(QVariant)));
    return m_Editor;
}

bool Vector4DProperty::setEditorData(QWidget *editor, const QVariant &data) {
    VectorEdit *e  = static_cast<VectorEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data.value<Vector4>());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant Vector4DProperty::editorData(QWidget *editor) {
    VectorEdit *e  = static_cast<VectorEdit *>(editor);
    if(e) {
        return QVariant::fromValue(e->data());
    }
    return Property::editorData(editor);
}

QSize Vector4DProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void Vector4DProperty::onDataChanged(const QVariant &data) {
    if(data.isValid()) {
        setValue(data);
    }
}
