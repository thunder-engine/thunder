#ifndef FILEPATHPROPERTY_H
#define FILEPATHPROPERTY_H

#include "Property.h"

struct FilePath {
    FilePath    () {}
    FilePath    (QString p) : path(p) {}
    QString     path;
};

Q_DECLARE_METATYPE(FilePath)

class FilePathProperty : public Property {
    Q_OBJECT
public:
    FilePathProperty                    (const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0);

    QVariant            value           (int role = Qt::UserRole) const;
    void                setValue        (const QVariant& value);

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem& option);

public slots:
    void                onFileDilog    ();

};

#endif // FILEPATHPROPERTY_H
