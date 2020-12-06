#ifndef LOCALEPROPERTY_H
#define LOCALEPROPERTY_H

#include "Property.h"

class LocaleProperty : public Property {
    Q_OBJECT

public:
    LocaleProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option);

    bool setEditorData(QWidget *editor, const QVariant &data);

    QVariant editorData(QWidget *editor);

protected slots:
    void valueChanged();

private:
    QSize sizeHint(const QSize &size) const;

    static QList<QLocale> m_locales;

};

#endif // LOCALEPROPERTY_H
