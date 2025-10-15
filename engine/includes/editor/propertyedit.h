#ifndef PROPERTYEDIT_H
#define PROPERTYEDIT_H

#include <QWidget>

#include <engine.h>

class ENGINE_EXPORT PropertyEdit : public QWidget {
    Q_OBJECT

public:
    typedef PropertyEdit*(*UserTypeCallback)(int userType, QWidget *parent, const TString &editor);

public:
    explicit PropertyEdit(QWidget *parent = nullptr);
    ~PropertyEdit();

    virtual Variant data() const;
    virtual void setData(const Variant &data);

    virtual void setEditorHint(const TString &hint);

    virtual void setObject(Object *object, const TString &);

    static void registerEditorFactory(UserTypeCallback callback);

    static void unregisterEditorFactory(UserTypeCallback callback);

    static PropertyEdit *constructEditor(int userType, QWidget *parent, const TString &editor);

signals:
    void dataChanged();
    void editFinished();

protected:
    static std::list<UserTypeCallback> m_userCallbacks;



    Object *m_object;

};

#endif // PROPERTYEDIT_H
