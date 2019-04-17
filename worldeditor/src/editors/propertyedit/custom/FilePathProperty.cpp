#include "FilePathProperty.h"

#include "../editors/PathEdit.h"

FilePathProperty::FilePathProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {
}

QVariant FilePathProperty::value(int role) const {
    QVariant data   = Property::value();
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

QWidget *FilePathProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option);
    QWidget *editor = new PathEdit(parent);
    connect(editor, SIGNAL(pathChanged(QFileInfo)), this, SLOT(onPathChanged(QFileInfo)));
    return editor;
}

bool FilePathProperty::setEditorData(QWidget *editor, const QVariant &data) {
    PathEdit *e = dynamic_cast<PathEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data.toString());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant FilePathProperty::editorData(QWidget *editor) {
    PathEdit *e = dynamic_cast<PathEdit *>(editor);
    if(e) {
        return QVariant(e->data());
    }
    return Property::editorData(editor);
}

QSize FilePathProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void FilePathProperty::onPathChanged(const QFileInfo &info) {
    setValue(QVariant::fromValue( info ));
}
