#ifndef PROPERTYEDIT_H
#define PROPERTYEDIT_H

#include <QWidget>

#include <engine.h>

class ENGINE_EXPORT PropertyEdit : public QWidget {
    Q_OBJECT

public:
    typedef PropertyEdit*(*UserTypeCallback)(int userType, QWidget *parent, const QString &name, QObject *object);

public:
    explicit PropertyEdit(QWidget *parent = nullptr);

    virtual QVariant data() const;
    virtual void setData(const QVariant &data);

    virtual void setEditorHint(const QString &hint);

    virtual void setObject(QObject *object, const QString &name);

    static void registerEditorFactory(UserTypeCallback callback);

    static void unregisterEditorFactory(UserTypeCallback callback);

    static PropertyEdit *constructEditor(int userType, QWidget *parent, const QString &name, QObject *object);

signals:
    void dataChanged();
    void editFinished();

protected:
    static QList<UserTypeCallback> m_userCallbacks;

    QString m_propertyName;

    QObject *m_propertyObject;

};

#endif // PROPERTYEDIT_H
