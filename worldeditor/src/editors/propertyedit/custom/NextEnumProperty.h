#ifndef NEXTENUMPROPERTY_H
#define NEXTENUMPROPERTY_H

#include <QStringList>

#include "Property.h"

#include <metaenum.h>

class Object;

struct Enum {
    Enum() :
        m_Object(nullptr),
        m_Value(0) {

    }

    QString m_EnumName;
    Object *m_Object;
    int32_t m_Value;
};
Q_DECLARE_METATYPE(Enum);

class NextEnumProperty : public Property {
    Q_OBJECT

public:
    NextEnumProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    virtual QVariant value(int role = Qt::UserRole) const;

    virtual QWidget *createEditor(QWidget* parent, const QStyleOptionViewItem &);

    virtual bool setEditorData(QWidget *editor, const QVariant &data);

    virtual QVariant editorData(QWidget *editor);

private slots:
    void valueChanged(const QString &item);

private:
    QStringList m_enum;

    Enum m_Value;

    MetaEnum m_metaEnum;

};

#endif // NEXTENUMPROPERTY_H
