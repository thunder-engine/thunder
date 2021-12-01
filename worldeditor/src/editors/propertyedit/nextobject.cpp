 #include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>
#include <QDebug>

#include <object.h>
#include <invalid.h>

#include "custom/AlignmentProperty.h"
#include "custom/AxisesProperty.h"
#include "custom/AssetProperty.h"
#include "custom/ColorProperty.h"
#include "custom/Vector4DProperty.h"
#include "custom/FilePathProperty.h"
#include "custom/ComponentProperty.h"
#include "custom/LocaleProperty.h"
#include "custom/NextEnumProperty.h"

#include <engine.h>
#include <components/scene.h>
#include <components/component.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/textrender.h>
#include <resources/material.h>
#include <resources/mesh.h>
#include <resources/font.h>

#include "assetmanager.h"

#include "editors/componentselect/componentselect.h"

#include "controllers/objectctrl.h"

#include <QMap>
#include <QMenu>

#define RESOURCE "Resource"

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

Q_DECLARE_METATYPE(Alignment)
Q_DECLARE_METATYPE(Axises)

#define COLOR       "Color"
#define AXISES      "Axises"
#define ALIGNMENT   "Alignment"
#define COMPONENT   "Component"
#define TEMPLATE    "Template"

namespace  {
    const char *EditorTag("editor=");
    const char *EnumTag("enum=");
    const char *MetaTag("meta=");
    const char *ReadOnlyTag("ReadOnly");
}

NextObject::NextObject(QObject *parent) :
        QObject(parent),
        m_pObject(nullptr) {
}

QString NextObject::name() {
    if(m_pObject) {
        return m_pObject->name().c_str();
    }
    return QString();
}

void NextObject::setName(const QString &name) {
    if(m_pObject) {
        m_pObject->setName(qPrintable(name));
        emit updated();
    }
}

void NextObject::setObject(Object *object) {
    m_pObject = object;
    onUpdated();
}

QMenu *NextObject::menu(Object *obj) {
    QMenu *result = nullptr;
    if(obj == nullptr || dynamic_cast<Transform *>(obj) || dynamic_cast<Actor *>(obj)) {
        return result;
    }

    result = new QMenu();
    QAction *del = new QAction(tr("Remove Component"), this);
    del->setProperty(COMPONENT, obj->typeName().c_str());
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
    foreach(QByteArray it, dynamicPropertyNames()) {
        setProperty(it, QVariant());
    }

    if(m_pObject) {
        m_Flags.clear();
        buildObject(m_pObject);

        setObjectName(m_pObject->typeName().c_str());
    }
    emit updated();
}

void NextObject::onPropertyContextMenuRequested(QString property, const QPoint point) {
    QMenu menu;
    QAction *action = menu.addAction(tr("Insert Keyframe"), this, SLOT(onInsertKeyframe()));

    QVariant data = NextObject::property(qPrintable(property));
    int32_t type = data.userType();
    action->setEnabled((type == QMetaType::Bool || type == QMetaType::Int || type == QMetaType::Float ||
                        type == QMetaType::type("Vector3") || type == QMetaType::type("QColor")));

    action->setProperty("property", property);

    menu.exec(point);
}

void NextObject::onInsertKeyframe() {
    QAction *action = static_cast<QAction *>(sender());
    QStringList list = action->property("property").toString().split('/');
    emit changed({findChild(list)}, list.back());
}

void NextObject::onDeleteComponent() {
    emit deleteComponent(sender()->property(COMPONENT).toString());
}

void NextObject::buildObject(Object *object, const QString &path) {
    const MetaObject *meta = object->metaObject();

    QString p(path.isEmpty() ? "" : path + "/");

    blockSignals(true);
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        QString name(property.name());
        if(name.indexOf("enabled") == -1) {
            name = p + name;
            Variant data = property.read(object);

            setProperty(qPrintable(name), qVariant(data, property, object));

            if(property.table() && property.table()->annotation) {
                QString annotation(property.table()->annotation);
                QStringList list = annotation.split(',');
                foreach(QString it, list) {
                    int index = it.indexOf(ReadOnlyTag);
                    if(index > -1) {
                        m_Flags[name] = true;
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
            invalid->setName(tr("%1 (Invalid)").arg(invalid->typeName().c_str()).toStdString());
            setProperty( qPrintable(p + invalid->name().c_str() + "/"), QVariant(true) );
            blockSignals(false);
        } else if(dynamic_cast<Component *>(it)) {
            buildObject(it, p + QString::fromStdString(it->typeName()));
        }
    }
}

bool NextObject::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        QString name = ev->propertyName();
        QVariant value = property(qPrintable(name));
        if(value.isValid()) {
            QStringList list = name.split('/');
            if(m_pObject) {
                Object *o = findChild(list);
                QString propertyName = list.join('/');
                Variant current = o->property(qPrintable(propertyName));

                const MetaObject *meta = o->metaObject();
                int index = meta->indexOfProperty(qPrintable(propertyName));
                MetaProperty property = meta->property(index);

                Variant target = aVariant(value, current, property);

                if(target.isValid() && current != target) {
                    emit aboutToBeChanged({o}, propertyName, target);

                    emit changed({o}, propertyName);
                }
            }
        }
    }
    return false;
}

QString NextObject::propertyTag(const MetaProperty &property, const QString &tag) const {
    if(property.table() && property.table()->annotation) {
        QString annotation(property.table()->annotation);
        QStringList list = annotation.split(',');
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
    Object *parent = m_pObject;
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

bool NextObject::isReadOnly(const QString &key) const {
    return m_Flags.value(key, false);
}

QString NextObject::propertyHint(const QString &name) const {
    QStringList list = name.split('/');
    if(m_pObject) {
        Object *o = findChild(list);
        QString propertyName = list.join('/');
        Variant current = o->property(qPrintable(propertyName));

        const MetaObject *meta = o->metaObject();
        int index = meta->indexOfProperty(qPrintable(propertyName));
        MetaProperty property = meta->property(index);

        return propertyTag(property, MetaTag);
    }

    return QString();
}

QVariant NextObject::qVariant(Variant &value, const MetaProperty &property, Object *object) {
    QString editor = propertyTag(property, EditorTag);
    QString enumProperty = propertyTag(property, EnumTag);

    switch(value.userType()) {
        case MetaType::BOOLEAN: {
            return QVariant(value.toBool());
        }
        case MetaType::INTEGER: {
            int32_t intValue = value.toInt();
            if(editor == AXISES) {
                return QVariant::fromValue(static_cast<Axises>(intValue));
            } else if(editor == ALIGNMENT) {
                return QVariant::fromValue(static_cast<Alignment>(intValue));
            } else if(!enumProperty.isEmpty()) {
                Enum enumValue;
                enumValue.m_Value = value.toInt();
                enumValue.m_EnumName = enumProperty;
                enumValue.m_Object = object;
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
            Vector4 vectorValue = value.toVector4();
            if(editor == COLOR) {
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
        if(factory->first->canCastTo(RESOURCE) || (editor == TEMPLATE)) {
            return QVariant::fromValue(Template(Engine::reference(o).c_str(), value.userType()));
        } else {
            Actor *actor = static_cast<Actor *>(m_pObject);
            SceneComponent cmp;
            cmp.type = typeName;
            cmp.component = dynamic_cast<Component *>(o);
            cmp.actor = dynamic_cast<Actor *>(o);
            cmp.scene = actor->scene();
            return QVariant::fromValue(cmp);
        }
    }

    return QVariant();
}

Variant NextObject::aVariant(QVariant &value, Variant &current, const MetaProperty &property) {
    QString editor = propertyTag(property, EditorTag);
    QString enumProperty = propertyTag(property, EnumTag);

    switch(current.userType()) {
        case MetaType::BOOLEAN: {
            return Variant(value.toBool());
        }
        case MetaType::INTEGER: {
            if(!enumProperty.isEmpty()) {
                Enum enumValue = value.value<Enum>();
                return Variant(enumValue.m_Value);
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
            if(editor == COLOR) {
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
        if(factory->first->canCastTo(RESOURCE)) {
            Template p = value.value<Template>();
            if(!p.path.isEmpty()) {
                Object *m = Engine::loadResource<Object>(qPrintable(p.path));
                return Variant(current.userType(), &m);
            }
        } else {
            SceneComponent c = value.value<SceneComponent>();
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

Property *NextObject::createCustomProperty(const QString &name, QObject *propertyObject, Property *parent) {
    int userType = 0;
    if(propertyObject) {
        userType = propertyObject->property(qPrintable(name)).userType();
    }

    if(userType == QMetaType::type("Vector2"))
        return new Vector4DProperty(name, propertyObject, 2, parent);

    if(userType == QMetaType::type("Vector3"))
        return new Vector4DProperty(name, propertyObject, 3, parent);

    if(userType == QMetaType::type("Vector4"))
        return new Vector4DProperty(name, propertyObject, 4, parent);

    if(userType == QMetaType::type("QColor"))
        return new ColorProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("QFileInfo"))
        return new FilePathProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("QLocale"))
        return new LocaleProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Template"))
        return new TemplateProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("SceneComponent"))
        return new ComponentProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Alignment"))
        return new AlignmentProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Axises"))
        return new AxisesProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Enum"))
        return new NextEnumProperty(name, propertyObject, parent);

    return nullptr;
}
