#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <QWidget>
#include <QStyleOption>
#include <QVariant>

class Property : public QObject {
    Q_OBJECT

public:
    explicit Property(const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0, bool root = false);

    void setName(const QString &value) { m_name = value; }

    QString name() const;

    QWidget *editor() const { return m_editor; }

    QObject *propertyObject() const { return m_propertyObject; }

    bool isRoot() const { return m_root; }

    bool isReadOnly() const;

    bool isCheckable() const { return m_checkable; }

    virtual bool isPersistent() const { return true; }

    int row() { return parent()->children().indexOf(this); }

    QString editorHints() const { return m_hints; }

    virtual QVariant value(int role = Qt::UserRole) const;

    virtual void setValue(const QVariant &value);

    virtual void setEditorHints(const QString &hints);

    virtual QVariant editorData(QWidget *);

    QWidget *getEditor(QWidget *parent) const;

    virtual bool setEditorData(QWidget *, const QVariant &);

    virtual QSize sizeHint(const QSize &size) const;

    virtual void setChecked(bool value);
    virtual bool isChecked() const;

    void setOverride(const QString &property);

protected:
    virtual QWidget *createEditor(QWidget *parent) const;

    QObject *m_propertyObject;
    QString m_hints;
    QString m_name;
    QString m_override;

    mutable QWidget *m_editor;

    bool m_root;
    bool m_checkable;
};

#endif
