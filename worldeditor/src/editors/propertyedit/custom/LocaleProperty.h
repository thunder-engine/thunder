#ifndef LOCALEPROPERTY_H
#define LOCALEPROPERTY_H

#include <editor/property.h>

class LocaleProperty : public Property {
    Q_OBJECT

public:
    explicit LocaleProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void valueChanged();

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

    static QList<QLocale> m_locales;

};

#endif // LOCALEPROPERTY_H
