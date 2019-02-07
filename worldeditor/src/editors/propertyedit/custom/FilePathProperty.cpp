#include "FilePathProperty.h"

#include "../editors/PathEdit.h"

#include <QFileDialog>

FilePathProperty::FilePathProperty(const QString &name /*= QString()*/, QObject *propertyObject /*= 0*/, QObject *parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant FilePathProperty::value(int role) const {
    QVariant data   = Property::value();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QString("%1").arg(data.value<QFileInfo>().absoluteFilePath());
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
    QWidget *pEditor = new PathEdit(parent);
    connect(pEditor, SIGNAL(pathChanged(QFileInfo)), this, SLOT(onPathChanged(QFileInfo)));
    return pEditor;
}

bool FilePathProperty::setEditorData(QWidget *editor, const QVariant &data) {
    PathEdit *e = dynamic_cast<PathEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setText(data.toString());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant FilePathProperty::editorData(QWidget *editor) {
    PathEdit *e = dynamic_cast<PathEdit *>(editor);
    if(e) {
        return QVariant(e->text());
    }
    return Property::editorData(editor);
}

void FilePathProperty::onPathChanged(const QFileInfo &info) {
    setValue(QVariant::fromValue( info ));
}
