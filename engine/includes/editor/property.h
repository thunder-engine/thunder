#ifndef PROPERTY_H
#define PROPERTY_H

#include <QVariant>

#include <engine.h>

class QWidget;

class ENGINE_EXPORT Property : public QObject {
    Q_OBJECT

public:
    typedef Property*(*UserTypeCallback)(const QString &name, QObject *propertyObject, Property *parent, bool root);

public:
    explicit Property(const QString &name = QString(), QObject *propertyObject = 0, QObject *parent = 0, bool root = false);

    QString name() const;
    void setName(const QString &value);

    QString editorHints() const;

    QWidget *getEditor(QWidget *parent) const;
    QWidget *editor() const;
    QObject *propertyObject() const;

    virtual bool isRoot() const;
    virtual bool isReadOnly() const;
    virtual bool isCheckable() const;
    virtual bool isPersistent() const;

    virtual QVariant value(int role = Qt::UserRole) const;
    virtual void setValue(const QVariant &value);

    virtual void setEditorHints(const QString &hints);

    virtual QVariant editorData(QWidget *editor);

    virtual bool setEditorData(QWidget *editor, const QVariant &data);

    virtual QSize sizeHint(const QSize &size) const;

    virtual bool isChecked() const;
    virtual void setChecked(bool value);

    static Property *constructProperty(const QString &name, QObject *propertyObject, Property *parent, bool root);
    static void registerPropertyFactory(UserTypeCallback callback);
    static void unregisterPropertyFactory(UserTypeCallback callback);

protected slots:
    void onDataChanged();

protected:
    virtual QWidget *createEditor(QWidget *parent) const;

protected:
    static QList<UserTypeCallback> m_userCallbacks;

    QObject *m_propertyObject;
    QString m_hints;
    QString m_name;

    mutable QWidget *m_editor;

    bool m_root;
    bool m_checkable;

};

#endif // PROPERTY_H
