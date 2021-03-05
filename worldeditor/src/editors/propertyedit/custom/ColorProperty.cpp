#include "ColorProperty.h"

#include "editors/propertyedit/editors/ColorEdit.h"
#include "../nextobject.h"

ColorProperty::ColorProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant ColorProperty::value(int role) const {
    QVariant data = Property::value(role);
    if(data.isValid()) {
        switch(role) {
            case Qt::DisplayRole: {
                return data;
            }
            case Qt::ToolTipRole: {
                return data.value<QColor>().name(QColor::HexArgb);
            }
            default: break;
        }
    }
    return data;
}

void ColorProperty::setValue(const QVariant &value) {
    if (value.type() == QVariant::String) {
        QString name    = value.toString();
        Property::setValue(QVariant::fromValue( QColor(name) ));
    } else {
        Property::setValue(value);
    }
}

QWidget *ColorProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    m_Editor = new ColorEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_Editor, SIGNAL(colorChanged(QString)), this, SLOT(onColorChanged(QString)));
    return m_Editor;
}

bool ColorProperty::setEditorData(QWidget *editor, const QVariant &data) {
    ColorEdit *e  = dynamic_cast<ColorEdit *>(editor);
    if(e) {
        QColor color    = data.value<QColor>();
        e->blockSignals(true);
        e->setColor(color.name(QColor::HexArgb));
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant ColorProperty::editorData(QWidget *editor) {
    ColorEdit *e  = dynamic_cast<ColorEdit *>(editor);
    if(e) {
        return e->color();
    }
    return Property::editorData(editor);
}

void ColorProperty::onColorChanged(const QString &color) {
    setValue(color);
}

QSize ColorProperty::sizeHint(const QSize& size) const {
    return size;
}

