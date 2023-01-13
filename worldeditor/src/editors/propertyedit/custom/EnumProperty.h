#ifndef ENUMPROPERTY_H
#define ENUMPROPERTY_H

#include <QStringList>

#include <editor/property.h>

class EnumProperty : public Property {
    Q_OBJECT

public:
    explicit EnumProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

private slots:
    void valueChanged(const QString &item);

private:
    QVariant value(int role = Qt::UserRole) const override;

    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

    QStringList m_enum;

};

#endif // ENUMPROPERTY_H
