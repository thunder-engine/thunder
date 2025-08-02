#ifndef PROPERTYEDIT_H
#define PROPERTYEDIT_H

#include <QWidget>

#include <engine.h>

class ENGINE_EXPORT PropertyEdit : public QWidget {
    Q_OBJECT

public:
    typedef PropertyEdit*(*UserTypeCallback)(int userType, QWidget *parent, const TString &name);

public:
    explicit PropertyEdit(QWidget *parent = nullptr);
    ~PropertyEdit();

    virtual QVariant data() const;
    virtual void setData(const QVariant &data);

    virtual void setEditorHint(const TString &hint);

    virtual void setObject(Object *object, const TString &name);

    static void registerEditorFactory(UserTypeCallback callback);

    static void unregisterEditorFactory(UserTypeCallback callback);

    static PropertyEdit *constructEditor(int userType, QWidget *parent, const TString &name);

signals:
    void dataChanged();
    void editFinished();

protected:
    static QList<UserTypeCallback> m_userCallbacks;

    TString m_propertyName;

    Object *m_object;

};

#endif // PROPERTYEDIT_H
