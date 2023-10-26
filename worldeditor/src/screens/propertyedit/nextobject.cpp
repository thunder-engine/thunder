#include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>
#include <QMenu>
#include <QMessageBox>

#include <object.h>
#include <invalid.h>

#include <engine.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/assetmanager.h>

#include "custom/array/arrayedit.h"
#include "custom/alignment/alignmentedit.h"
#include "custom/axises/axisesedit.h"
#include "custom/color/coloredit.h"
#include "custom/component/componentproperty.h"
#include "custom/locale/localeedit.h"
#include "custom/objectselect/objectselect.h"
#include "custom/filepath/pathedit.h"
#include "custom/nextenum/nextenumedit.h"
#include "custom/vector4/vector4edit.h"

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

Q_DECLARE_METATYPE(Alignment)
Q_DECLARE_METATYPE(Axises)

namespace  {
    const char *gEditorTag("editor=");
    const char *gEnumTag("enum=");
    const char *gMetaTag("meta=");
    const char *gReadOnlyTag("ReadOnly");

    const char *gAxises("Axises");
    const char *gColor("Color");
    const char *gAlignment("Alignment");
    const char *gResource("Resource");
    const char *gComponent("Component");
    const char *gAsset("Asset");
}

NextObject::NextObject(QObject *parent) :
        QObject(parent),
        m_object(nullptr) {

    setProperty("_next", true);

    Property::registerPropertyFactory(NextObject::createCustomProperty);

    PropertyEdit::registerEditorFactory(NextObject::createCustomEditor);
}

QString NextObject::name() {
    if(m_object) {
        return m_object->name().c_str();
    }
    return QString();
}

void NextObject::setName(const QString &name) {
    if(m_object) {
        m_object->setName(qPrintable(name));
        emit updated();
    }
}

void NextObject::setObject(Object *object) {
    m_object = object;
    onUpdated();
}

QMenu *NextObject::menu(Object *obj) {
    QMenu *result = nullptr;
    if(obj == nullptr || dynamic_cast<Transform *>(obj) || dynamic_cast<Actor *>(obj)) {
        return result;
    }

    result = new QMenu();
    QAction *del = new QAction(tr("Remove Component"), this);
    del->setProperty(gComponent, obj->typeName().c_str());
    result->addAction(del);

    connect(del, SIGNAL(triggered(bool)), this, SLOT(onDeleteComponent()));

    return result;
}

Object *NextObject::component(const QString &name) {
    QStringList path(name.split('/'));
    QStringList dir(path.mid(0, path.size()));
    return findChild(dir);
}

void NextObject::onUpdated() {
    for(QByteArray &it : dynamicPropertyNames()) {
        if(it != "_next") {
            setProperty(it, QVariant());
        }
    }

    if(m_object) {
        m_flags.clear();
        buildObject(m_object);

        setObjectName(m_object->typeName().c_str());
    }
}

void NextObject::onCreateComponent(QString type) {
    Actor *actor = dynamic_cast<Actor *>(m_object);
    if(actor) {
        if(actor->component(qPrintable(type)) == nullptr) {
            UndoManager::instance()->push(new CreateComponent(type, m_object, this));
        } else {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Creation Component Failed"));
            msgBox.setInformativeText(QString(tr("Component with type \"%1\" already defined for this actor.")).arg(type));
            msgBox.setStandardButtons(QMessageBox::Ok);

            msgBox.exec();
        }
    }
}

void NextObject::onDeleteComponent() {
    QString name(sender()->property(gComponent).toString());

    UndoManager::instance()->push(new RemoveComponent(component(name), this));
}

void NextObject::onObjectsChanged(QList<Object *> objects, const QString property, Variant value) {
    QUndoCommand *group = UndoManager::instance()->group();
    QString name;
    if(group == nullptr) {
        name = QObject::tr("Change Property");
    }
    UndoManager::instance()->push(new PropertyObject(objects.first(), property, value, this, name, group));
}

void NextObject::onPropertyContextMenuRequested(QString property, const QPoint point) {
    QMenu menu;
    QAction *action = menu.addAction(tr("Insert Keyframe"), this, SLOT(onInsertKeyframe()));

    QVariant data(NextObject::property(qPrintable(property)));
    int32_t type = data.userType();
    action->setEnabled((type == QMetaType::Bool ||
                        type == QMetaType::Int ||
                        type == QMetaType::Float ||
                        type == QMetaType::type("Vector3") ||
                        type == QMetaType::type("QColor")));

    action->setProperty("property", property);

    menu.exec(point);
}

void NextObject::onInsertKeyframe() {
    QString property = static_cast<QAction *>(sender())->property("property").toString();
    QStringList list(property.split('/'));

    emit propertyChanged({findChild(list)}, list.back(), Variant());
}

void NextObject::buildObject(Object *object, const QString &path) {
    const MetaObject *meta = object->metaObject();

    QString p(path.isEmpty() ? "" : path + "/");

    blockSignals(true);
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property(meta->property(i));
        QString name(property.name());
        if(name.indexOf("enabled") == -1) {
            name = p + name;
            Variant data(property.read(object));

            setProperty(qPrintable(name), qVariant(data, property, object));

            if(property.table() && property.table()->annotation) {
                QString annotation(property.table()->annotation);
                QStringList list(annotation.split(','));
                foreach(QString it, list) {
                    int index = it.indexOf(gReadOnlyTag);
                    if(index > -1) {
                        m_flags[name] = true;
                        break;
                    }
                }
            }

        } else {
            setProperty(qPrintable(p), QVariant(true));
        }
    }
    blockSignals(false);

    for(Object *it : object->getChildren()) {
        Invalid *invalid = dynamic_cast<Invalid *>(it);
        if(invalid) {
            blockSignals(true);
            invalid->setName(invalid->typeName());
            setProperty(qPrintable(p + invalid->name().c_str() + "/"), QVariant(true));
            blockSignals(false);
        } else if(dynamic_cast<Component *>(it)) {
            buildObject(it, p + QString::fromStdString(it->typeName()));
        }
    }
}

bool NextObject::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        QString name(ev->propertyName());
        QVariant value(property(qPrintable(name)));

        if(value.isValid() && m_object) {
            QStringList list(name.split('/'));
            Object *o = findChild(list);
            QString propertyName(list.join('/'));
            Variant current(o->property(qPrintable(propertyName)));

            const MetaObject *meta = o->metaObject();
            int index = meta->indexOfProperty(qPrintable(propertyName));
            MetaProperty property(meta->property(index));
            Variant target(aVariant(value, current, property));

            if(target.isValid() && current != target) {
                onObjectsChanged({o}, propertyName, target);
            }
        }
    }
    return false;
}

QString NextObject::propertyTag(const MetaProperty &property, const QString &tag) const {
    if(property.table() && property.table()->annotation) {
        QString annotation(property.table()->annotation);
        QStringList list(annotation.split(','));
        foreach(QString it, list) {
            int index = it.indexOf(tag);
            if(index > -1) {
                return it.remove(tag);
            }
        }
    }
    return QString();
}

Object *NextObject::findChild(QStringList &path) const {
    Object *parent = m_object;
    if(parent == nullptr) {
        return nullptr;
    }
    foreach(QString str, path) {
        for(Object *it : parent->getChildren()) {
            if(it->typeName() == str.toStdString()) {
                parent = it;
                path.pop_front();
                break;
            }
        }
    }
    return parent;
}

Object *NextObject::findById(uint32_t id, Object *parent) {
    if(m_object->uuid() == id) {
        return m_object;
    }

    Object *p = parent;
    if(p == nullptr) {
        p = m_object;
    }

    return ObjectSystem::findObject(id, p);
}

bool NextObject::isReadOnly(const QString &key) const {
    return m_flags.value(key, false);
}

QString NextObject::propertyHint(const QString &name) const {
    QStringList list(name.split('/'));
    if(m_object) {
        Object *o = findChild(list);
        QString propertyName(list.join('/'));

        const MetaObject *meta = o->metaObject();
        int index = meta->indexOfProperty(qPrintable(propertyName));
        MetaProperty property(meta->property(index));

        return propertyTag(property, gMetaTag);
    }

    return QString();
}

QVariant NextObject::qVariant(Variant &value, const MetaProperty &property, Object *object) {
    QString editor(propertyTag(property, gEditorTag));

    switch(value.userType()) {
        case MetaType::BOOLEAN: {
            return QVariant(value.toBool());
        }
        case MetaType::INTEGER: {
            QString enumProperty = propertyTag(property, gEnumTag);

            int32_t intValue = value.toInt();
            if(editor == gAxises) {
                return QVariant::fromValue(static_cast<Axises>(intValue));
            } else if(editor == gAlignment) {
                return QVariant::fromValue(static_cast<Alignment>(intValue));
            } else if(!enumProperty.isEmpty()) {
                Enum enumValue;
                enumValue.m_value = value.toInt();
                enumValue.m_enumName = enumProperty;
                enumValue.m_object = object;
                return QVariant::fromValue(enumValue);
            }
            return QVariant(value.toInt());
        }
        case MetaType::FLOAT: {
            return QVariant(value.toFloat());
        }
        case MetaType::STRING: {
            return QVariant(value.toString().c_str());
        }
        case MetaType::VECTOR2: {
            return QVariant::fromValue(value.toVector2());
        }
        case MetaType::VECTOR3: {
            return QVariant::fromValue(value.toVector3());
        }
        case MetaType::VECTOR4: {
            Vector4 vectorValue(value.toVector4());
            if(editor == gColor) {
                QColor r;
                r.setRgbF(vectorValue.x, vectorValue.y, vectorValue.z, vectorValue.w);
                return QVariant(r);
            }
            return QVariant::fromValue(vectorValue);
        }
        case MetaType::QUATERNION:
        case MetaType::MATRIX3:
        case MetaType::MATRIX4: {
            return QVariant();
        }
        default: break;
    }

    QString typeName(QString(property.type().name()).replace(" *", ""));
    auto factory = System::metaFactory(qPrintable(typeName));
    if(factory) {
        Object *o = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
        if(factory->first->canCastTo(gResource) || (editor == gAsset)) {
            return QVariant::fromValue(Template(Engine::reference(o).c_str(), value.userType()));
        } else {
            Scene *scene = nullptr;
            Actor *actor = dynamic_cast<Actor *>(object);
            Component *component = dynamic_cast<Component *>(object);

            if(actor) {
                scene = actor->scene();
            } else if(component) {
                scene = component->scene();
            }

            ObjectData cmp;
            cmp.type = typeName;
            cmp.component = dynamic_cast<Component *>(o);
            cmp.actor = dynamic_cast<Actor *>(o);
            cmp.scene = scene;

            return QVariant::fromValue(cmp);
        }
    }

    return QVariant();
}

Variant NextObject::aVariant(QVariant &value, Variant &current, const MetaProperty &property) {
    QString editor(propertyTag(property, gEditorTag));

    switch(current.userType()) {
        case MetaType::BOOLEAN: {
            return Variant(value.toBool());
        }
        case MetaType::INTEGER: {
            QString enumProperty = propertyTag(property, gEnumTag);
            if(!enumProperty.isEmpty()) {
                Enum enumValue = value.value<Enum>();
                return Variant(enumValue.m_value);
            }
            return Variant(value.toInt());
        }
        case MetaType::FLOAT: {
            return Variant(value.toFloat());
        }
        case MetaType::STRING: {
            if(value.canConvert<QFileInfo>()) {
                QFileInfo p = value.value<QFileInfo>();
                return Variant(qUtf8Printable(p.absoluteFilePath()));
            } else if(value.canConvert<Template>()) {
                Template p = value.value<Template>();
                return Variant(qUtf8Printable(p.path));
            }
            return Variant(qUtf8Printable(value.toString()));
        }
        case MetaType::VECTOR2: {
            return Variant(value.value<Vector2>());
        }
        case MetaType::VECTOR3: {
            return Variant(value.value<Vector3>());
        }
        case MetaType::VECTOR4: {
            if(editor == gColor) {
                QColor c = value.value<QColor>();
                return Variant(Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
            }
            return Variant(value.value<Vector4>());
        }
        default: break;
    }

    QString typeName(QString(property.type().name()).replace(" *", ""));
    auto factory = System::metaFactory(qPrintable(typeName));
    if(factory) {
        if(factory->first->canCastTo(gResource)) {
            Template p = value.value<Template>();
            if(!p.path.isEmpty()) {
                Object *m = Engine::loadResource<Object>(qPrintable(p.path));
                return Variant(current.userType(), &m);
            } else {
                return Variant(current.userType(), nullptr);
            }
        } else {
            ObjectData c(value.value<ObjectData>());
            if(c.component) {
                return Variant(current.userType(), &c.component);
            }
            if(c.actor) {
                return Variant(current.userType(), &c.actor);
            }
            return Variant(current.userType(), nullptr);
        }
    }

    return Variant();
}

Property *NextObject::createCustomProperty(const QString &name, QObject *propertyObject, Property *parent, bool root) {
    NextObject *next = dynamic_cast<NextObject *>(propertyObject);
    if(next && root) {
        return new ComponentProperty(name, propertyObject, parent, root);
    }
    return nullptr;
}

PropertyEdit *NextObject::createCustomEditor(int userType, QWidget *parent, const QString &name, QObject *object) {
    PropertyEdit *result = nullptr;

    if(userType == QMetaType::QVariantList) {
        result = new ArrayEdit(parent);
    } else if(userType == qMetaTypeId<Vector2>() ||
       userType == qMetaTypeId<Vector3>() ||
       userType == qMetaTypeId<Vector4>()) {
        result = new Vector4Edit(parent);
    } else if(userType == qMetaTypeId<Enum>()) {
        result = new NextEnumEdit(parent);
    } else if(userType == qMetaTypeId<QFileInfo>()) {
        result = new PathEdit(parent);
    } else if(userType == qMetaTypeId<QLocale>()) {
        result = new LocaleEdit(parent);
    } else if(userType == qMetaTypeId<Axises>()) {
        result = new AxisesEdit(parent);
    } else if(userType == qMetaTypeId<Alignment>()) {
        result = new AlignmentEdit(parent);
    } else if(userType == qMetaTypeId<QColor>()) {
        result = new ColorEdit(parent);
    } else if(userType == qMetaTypeId<Template>() ||
              userType == qMetaTypeId<ObjectData>()) {
        result = new ObjectSelect(parent);
    }

    NextObject *next = dynamic_cast<NextObject *>(object);
    if(next && result) {
        result->setEditorHint(next->propertyHint(name));
    }

    return result;
}

PropertyObject::PropertyObject(Object *object, const QString &property, const Variant &value, NextObject *next, const QString &name, QUndoCommand *group) :
        UndoCommand(name, next, group),
        m_next(next),
        m_value(value),
        m_property(property),
        m_object(object->uuid()) {

}
void PropertyObject::undo() {
    PropertyObject::redo();
}
void PropertyObject::redo() {
    Variant value(m_value);

    Scene *scene = nullptr;

    Object *object = m_next->findById(m_object);
    if(object) {
        const MetaObject *meta = object->metaObject();
        int index = meta->indexOfProperty(qPrintable(m_property));
        if(index > -1) {
            MetaProperty property = meta->property(index);
            if(property.isValid()) {
                m_value = property.read(object);

                property.write(object, value);
            }
        }
    }

    m_next->onUpdated();
}

RemoveComponent::RemoveComponent(const Object *component, NextObject *next, const QString &name, QUndoCommand *group) :
        UndoCommand(name + " " + component->typeName().c_str(), next, group),
        m_next(next),
        m_parent(0),
        m_uuid(component->uuid()),
        m_index(0) {

}
void RemoveComponent::undo() {
    Scene *scene = nullptr;

    Object *parent = m_next->findById(m_parent);
    Object *object = Engine::toObject(m_dump, parent);
    if(object) {
        object->setParent(parent, m_index);

        emit m_next->structureChanged();
    }
}
void RemoveComponent::redo() {
    Scene *scene = nullptr;

    m_dump = Variant();
    m_parent = 0;
    Object *object = m_next->findById(m_uuid);
    if(object) {
        m_dump = Engine::toVariant(object, true);
        m_parent = object->parent()->uuid();

        QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
        m_index = children.indexOf(object);

        delete object;

        emit m_next->structureChanged();
    }
}

CreateComponent::CreateComponent(const QString &type, Object *object, NextObject *next, QUndoCommand *group) :
        UndoCommand(QObject::tr("Create %1").arg(type), next, group),
        m_next(next),
        m_type(type),
        m_object(object->uuid()) {

}
void CreateComponent::undo() {
    for(auto uuid : m_objects) {
        Object *object = m_next->findById(uuid);
        if(object) {
            delete object;

            emit m_next->structureChanged();
        }
    }
}
void CreateComponent::redo() {
    Object *object = m_next->findById(m_object);

    if(object) {
        Component *component = dynamic_cast<Component *>(Engine::objectCreate(qPrintable(m_type), qPrintable(m_type), object));
        if(component) {
            component->composeComponent();
            m_objects.push_back(component->uuid());

            emit m_next->structureChanged();
        }
    }
}
