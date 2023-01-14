#include "filepathproperty.h"

#include "pathedit.h"

FilePathProperty::FilePathProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QVariant FilePathProperty::value(int role) const {
    QVariant data = Property::value(role);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return data.value<QFileInfo>().absoluteFilePath();
    }
    return data;
}

void FilePathProperty::setValue(const QVariant &value) {
    if (value.type() == QVariant::String) {
        Property::setValue(QVariant::fromValue( QFileInfo(value.toString()) ));
    } else {
        Property::setValue(value);
    }
}

QWidget *FilePathProperty::createEditor(QWidget *parent) const {
    m_editor = new PathEdit(parent);
    m_editor->setDisabled(isReadOnly());
    connect(m_editor, SIGNAL(pathChanged(QFileInfo)), this, SLOT(onPathChanged(QFileInfo)));
    return m_editor;
}

bool FilePathProperty::setEditorData(QWidget *editor, const QVariant &data) {
    PathEdit *e = static_cast<PathEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data.toString());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant FilePathProperty::editorData(QWidget *editor) {
    PathEdit *e = static_cast<PathEdit *>(editor);
    if(e) {
        return QVariant::fromValue(e->data());
    }
    return Property::editorData(editor);
}

void FilePathProperty::onPathChanged(const QFileInfo &info) {
    setValue(QVariant::fromValue(info));
}
