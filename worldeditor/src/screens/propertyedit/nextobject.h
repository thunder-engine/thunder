#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>
#include <QHash>
#include <QPoint>

#include <object.h>
#include <editor/undomanager.h>

class QMenu;
class Property;
class PropertyEdit;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit NextObject(QObject *parent = nullptr);

    QString name();
    void setName(const QString &name);

    void setObject(Object *object);

    QMenu *menu(Object *obj);

    Object *component(const QString &name);
    Object *findById(uint32_t id, Object *parent = nullptr);

    bool isReadOnly(const QString &key) const;

    QString propertyHint(const QString &name) const;

    static Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent, bool root);

    static PropertyEdit *createCustomEditor(int userType, QWidget *parent, const QString &name, QObject *object);

public slots:
    void onUpdated();
    void onCreateComponent(QString type);

    void onPropertyContextMenuRequested(QString property, const QPoint point);

    void onInsertKeyframe();

signals:
    void updated();

    void propertyChanged(QList<Object *> objects, const QString property, Variant value);

    void structureChanged(QList<Object *> objects, bool force = false);

protected slots:
    void onDeleteComponent();

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

class RemoveComponent : public UndoCommand {
public:
    RemoveComponent(const Object *component, NextObject *next, const QString &name = QObject::tr("Remove Component"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    NextObject *m_next;

    Variant m_dump;
    uint32_t m_parent;
    uint32_t m_uuid;
    int32_t m_index;

};

class CreateComponent : public UndoCommand {
public:
    CreateComponent(const QString &type, Object *object, NextObject *next, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    NextObject *m_next;

    std::list<uint32_t> m_objects;
    QString m_type;
    uint32_t m_object;

};

#endif // NEXTOBJECT_H
