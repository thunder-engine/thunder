#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>
#include <QHash>

#include <object.h>
#include <editor/undomanager.h>

class PropertyEdit;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit NextObject(QObject *parent = nullptr);

    QString name();
    void setName(const QString &name);

    void setObject(Object *object);

    Object *component(const QString &name);
    Object *findById(uint32_t id, Object *parent = nullptr);

    bool isReadOnly(const QString &key) const;

    QString propertyHint(const QString &name) const;

    static PropertyEdit *createCustomEditor(int userType, QWidget *parent, const QString &name, QObject *object);

public slots:
    void onUpdated();

signals:
    void updated();

    void propertyChanged(QList<Object *> objects, const QString property, Variant value);

    void structureChanged(QList<Object *> objects, bool force = false);

protected:
    bool event(QEvent *e);

    Object *findChild(QStringList &path) const;

    QString propertyTag(const MetaProperty &property, const QString &tag) const;

    QVariant qVariant(const Variant &value, const MetaProperty &property, Object *object);
    Variant aVariant(const QVariant &value, const Variant &current, const MetaProperty &property);

    void buildObject(Object *object, const QString &path = QString());

    QVariant qObjectVariant(const Variant &value, const std::string &typeName, const QString &editor);
    Variant aObjectVariant(const QVariant &value, uint32_t type, const std::string &typeName);

protected:
    QHash<QString, bool> m_flags;

    Object *m_object;

};

#endif // NEXTOBJECT_H
