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
    explicit NextObject(Object *data, QObject *parent = nullptr);

    QString name();
    void setName(const QString &name);

    QMenu *menu(Object *obj);

    Object *component(const QString &name);

    Object *findChild(QStringList &path);

    bool isReadOnly(const QString &key) const;

    static Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent);

public slots:
    void onUpdated();

signals:
    void aboutToBeChanged(Object *object, const QString property, const Variant &value);
    void changed(Object *object, const QString property);

    void updated();

    void deleteComponent(const QString name);

protected slots:
    void onDeleteComponent();

protected:
    bool event(QEvent *e);

    QString editorTag(const MetaProperty &property);
    QString enumTag(const MetaProperty &property);

    QVariant qVariant(Variant &value, const MetaProperty &property, Object *object);
    Variant aVariant(QVariant &value, Variant &current, const MetaProperty &property);

    void buildObject(Object *object, const QString &path = QString());

    Object *m_pObject;
    QHash<QString, bool> m_Flags;

};

#endif // NEXTOBJECT_H
