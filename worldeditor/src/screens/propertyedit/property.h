#ifndef PROPERTY_H
#define PROPERTY_H

#include <QVariant>

#include <engine.h>

class QWidget;

class Property : public QObject {
    Q_OBJECT

public:
    explicit Property(const QString &name, Property *parent, bool root, bool second);

    void setPropertyObject(QObject *propertyObject);
    void setPropertyObject(Object *propertyObject);

    QString name() const;
    void setName(const QString &value);

    QString editorHints() const;

    QWidget *getEditor(QWidget *parent) const;
    QWidget *editor() const;
    QObject *propertyObject() const;

    virtual bool isRoot() const;
    virtual bool isReadOnly() const;

    virtual QVariant value(int role = Qt::UserRole) const;
    virtual void setValue(const QVariant &value);

    virtual void setEditorHints(const QString &hints);

    virtual QVariant editorData(QWidget *editor);

    virtual bool setEditorData(QWidget *editor, const QVariant &data);

    virtual QSize sizeHint(const QSize &size) const;

    virtual bool isCheckable() const;
    virtual bool isChecked() const;
    virtual void setChecked(bool value);

protected slots:
    void onDataChanged();
    void onEditorDestoyed();

protected:
    virtual QWidget *createEditor(QWidget *parent) const;

protected:
    QObject *m_propertyObject;
    Object *m_nextObject;

    QString m_hints;
    QString m_name;

    mutable QWidget *m_editor;

    bool m_root;
    bool m_second;
    bool m_checkable;

};

#endif // PROPERTY_H
