#include "Vector3DProperty.h"

#include "editors/propertyedit/editors/VectorEdit.h"

Vector3DProperty::Vector3DProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant Vector3DProperty::value(int role) const {
    QVariant data = Property::value(role);
    if(data.isValid() && role != Qt::UserRole) {
        switch(role) {
            case Qt::DisplayRole: {
                return QVariant();
            }
            default: break;
        }
    }
    return data;
}

QWidget *Vector3DProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    m_Editor = new VectorEdit(parent);
    connect(m_Editor, SIGNAL(dataChanged(QVariant)), this, SLOT(onDataChanged(QVariant)));
    return m_Editor;
}

bool Vector3DProperty::setEditorData(QWidget *editor, const QVariant &data) {
    VectorEdit *e  = dynamic_cast<VectorEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data.value<Vector3>());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant Vector3DProperty::editorData(QWidget *editor) {
    VectorEdit *e  = dynamic_cast<VectorEdit *>(editor);
    if(e) {
        return QVariant::fromValue(e->data());
    }
    return Property::editorData(editor);
}

QSize Vector3DProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void Vector3DProperty::onDataChanged(const QVariant &data) {
    if(data.isValid()) {
        setValue(data);
    }
}
