#ifndef FILEPATHPROPERTY_H
#define FILEPATHPROPERTY_H

#include "Property.h"

class QFileInfo;

class FilePathProperty : public Property {
    Q_OBJECT
public:
    explicit FilePathProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

private slots:
    void onPathChanged(const QFileInfo &info);

private:
    QVariant value(int role = Qt::UserRole) const override;
    void setValue(const QVariant &value) override;

    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // FILEPATHPROPERTY_H
