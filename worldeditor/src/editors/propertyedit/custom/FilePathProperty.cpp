#include "FilePathProperty.h"

#include "../editors/PathEdit.h"

#include <QFileDialog>

FilePathProperty::FilePathProperty(const QString &name /*= QString()*/, QObject *propertyObject /*= 0*/, QObject *parent /*= 0*/) :
        Property(name, propertyObject, parent) {
}

QVariant FilePathProperty::value(int role) const {
    QVariant data   = Property::value();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QString("%1").arg(data.value<FilePath>().path);
    }
    return data;
}

void FilePathProperty::setValue(const QVariant &value) {
    if (value.type() == QVariant::String) {
        Property::setValue(QVariant::fromValue( FilePath(value.toString()) ));
    } else {
        Property::setValue(value);
    }
}

QWidget *FilePathProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    QWidget *pEditor = new PathEdit(parent);
    connect(pEditor, SIGNAL(openFileDlg()), this, SLOT(onFileDilog()));
    return pEditor;
}

void FilePathProperty::onFileDilog() {
    QString mDir    = QDir::currentPath();

    QString mPath   = QFileDialog::getOpenFileName( dynamic_cast<QWidget *>(parent()),
                                                    tr("Select File"),
                                                    mDir,
                                                    tr("All Files (*.*)"));

    if(mPath.length() > 0) {
        QDir path   = QDir(mDir);
        mPath       = path.relativeFilePath(mPath);

        setValue(mPath);
    }
}
