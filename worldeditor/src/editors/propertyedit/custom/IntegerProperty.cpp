#include "IntegerProperty.h"

#include "../editors/IntegerEdit.h"
#include "../nextobject.h"

IntegerProperty::IntegerProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *IntegerProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    IntegerEdit *editor = new IntegerEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        editor->setDisabled(object->isReadOnly(objectName()));
        if(!m_hints.isEmpty()) {
            static QRegExp regExp {"\\d+"};

            QStringList list;
            int pos = 0;

            while((pos = regExp.indexIn(m_hints, pos)) != -1) {
                list << regExp.cap(0);
                pos += regExp.matchedLength();
            }

            if(list.size() == 2) {
                editor->setInterval(list[0].toInt(), list[1].toInt());
            }
        }
    }

    m_editor = editor;

    connect(m_editor, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
    return m_editor;
}

bool IntegerProperty::setEditorData(QWidget *editor, const QVariant &data) {
    IntegerEdit *e = static_cast<IntegerEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setValue(data.toInt());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant IntegerProperty::editorData(QWidget *editor) {
    IntegerEdit *e = static_cast<IntegerEdit *>(editor);
    if(e) {
        return QVariant(e->value());
    }
    return Property::editorData(editor);
}

void IntegerProperty::onDataChanged() {
    IntegerEdit *e = dynamic_cast<IntegerEdit *>(m_editor);
    if(e) {
        setValue(QVariant(e->value()));
    }
}

QSize IntegerProperty::sizeHint(const QSize &size) const {
    return QSize(size.width(), 27);
}
