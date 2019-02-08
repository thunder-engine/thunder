#ifndef FILEPATHPROPERTY_H
#define FILEPATHPROPERTY_H

#include "Property.h"

#include <QFileInfo>

class FilePathProperty : public Property {
    Q_OBJECT
public:
    FilePathProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QVariant value(int role = Qt::UserRole) const;
    void setValue(const QVariant& value);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem& option);

    bool setEditorData(QWidget *editor, const QVariant &data);

    QVariant editorData(QWidget *editor);

private slots:
    void onPathChanged(const QFileInfo &info);

private:
    QSize sizeHint(const QSize& size) const;

};

#endif // FILEPATHPROPERTY_H
