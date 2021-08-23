#include "FloatProperty.h"

#include "../editors/FloatEdit.h"
#include "../nextobject.h"

FloatProperty::FloatProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *FloatProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    FloatEdit *editor = new FloatEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        editor->setDisabled(object->isReadOnly(objectName()));
        if(!m_hints.isEmpty()) {
            static QRegExp regExp {"\\d+\\.\\d+"};

            QStringList list;
            int pos = 0;

            while((pos = regExp.indexIn(m_hints, pos)) != -1) {
                list << regExp.cap(0);
                pos += regExp.matchedLength();
            }

            if(list.size() == 2) {
                editor->setInterval(list[0].toFloat(), list[1].toFloat());
            }
        }
    }
    m_editor = editor;
    connect(m_editor, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
    return m_editor;
}

bool FloatProperty::setEditorData(QWidget *editor, const QVariant &data) {
    FloatEdit *e = static_cast<FloatEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setValue(data.toDouble());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant FloatProperty::editorData(QWidget *editor) {
    FloatEdit *e = static_cast<FloatEdit *>(editor);
    if(e) {
        return QVariant(e->value());
    }
    return Property::editorData(editor);
}

void FloatProperty::onDataChanged() {
    FloatEdit *e = static_cast<FloatEdit *>(m_editor);
    if(e) {
        setValue(QVariant(e->value()));
    }
}

QSize FloatProperty::sizeHint(const QSize &size) const {
    return QSize(size.width(), 27);
}
