#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>
#include <QHash>

#include <object.h>

#include "custom/Property.h"

class QMenu;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit NextObject(QObject *parent = nullptr);

    QString name();
    void setName(const QString &name);

    void setObject(Object *object);

    QMenu *menu(Object *obj);

    Object *component(const QString &name);



    bool isReadOnly(const QString &key) const;

    QString propertyHint(const QString &name) const;

    static Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent);

public slots:
    void onUpdated();

    void onPropertyContextMenuRequested(QString property, const QPoint point);

    void onInsertKeyframe();

signals:
    void aboutToBeChanged(Object::ObjectList objects, const QString property, const Variant &value);
    void changed(Object::ObjectList objects, const QString property);

    void updated();

    void deleteComponent(const QString name);

protected slots:
    void onDeleteComponent();

protected:
    bool event(QEvent *e);

    Object *findChild(QStringList &path) const;

    QString propertyTag(const MetaProperty &property, const QString &tag) const;

    QVariant qVariant(Variant &value, const MetaProperty &property, Object *object);
    Variant aVariant(QVariant &value, Variant &current, const MetaProperty &property);

    void buildObject(Object *object, const QString &path = QString());

    Object *m_pObject;
    QHash<QString, bool> m_Flags;

};

#endif // NEXTOBJECT_H
