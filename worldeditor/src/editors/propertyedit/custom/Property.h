#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <QWidget>
#include <QStyleOption>
#include <QVariant>

class Property : public QObject {
    Q_OBJECT

public:

    Property(const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0, bool root = false);

    void setName(const QString &value) { m_name = value; }

    QString name() const;

    QWidget *editor() const { return m_Editor; }

    QObject *propertyObject() const { return m_propertyObject; }

    bool isRoot() const { return m_Root; }

    bool isReadOnly() const;

    bool isCheckable() const { return isRoot(); }

    virtual bool isPersistent() const { return true; }

    int row() { return parent()->children().indexOf(this); }

    QString editorHints() const { return m_hints; }

    virtual QVariant value(int role = Qt::UserRole) const;

    virtual void setValue(const QVariant &value);

    virtual void setEditorHints(const QString &hints) { m_hints = hints; }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &);

    virtual QVariant editorData(QWidget *);

    virtual bool setEditorData(QWidget *, const QVariant &);

    virtual QSize sizeHint(const QSize &size) const;

    virtual void setChecked(bool value);
    virtual bool isChecked() const;

    Property *findPropertyObject(QObject *propertyObject);

protected:
    QObject *m_propertyObject;
    QString m_hints;
    QString m_name;

    QWidget *m_Editor;

    bool m_Root;
    bool m_Checkable;
};

#endif
