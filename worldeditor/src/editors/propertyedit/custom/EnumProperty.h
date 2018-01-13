#ifndef ENUMPROPERTY_H
#define ENUMPROPERTY_H

#include <QStringList>

#include "Property.h"

class EnumProperty : public Property {
    Q_OBJECT

public:
    EnumProperty            (const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0);

    virtual QVariant        value           (int role = Qt::UserRole) const;

    virtual QWidget        *createEditor    (QWidget* parent, const QStyleOptionViewItem &);

    virtual bool            setEditorData   (QWidget *editor, const QVariant &data);

    virtual QVariant        editorData      (QWidget *editor);

private slots:
    void                    valueChanged    (const QString &item);

private:
    QStringList             m_enum;

};

#endif // ENUMPROPERTY_H
