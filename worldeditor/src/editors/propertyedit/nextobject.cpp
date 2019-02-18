#include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>

#include <object.h>
#include <invalid.h>

#include "custom/Vector3DProperty.h"
#include "custom/ColorProperty.h"
#include "custom/FilePathProperty.h"
#include "custom/AssetProperty.h"

#include "managers/undomanager/undomanager.h"

#include <engine.h>
#include <components/component.h>
#include <components/actor.h>
#include <components/transform.h>
#include <resources/material.h>
#include <resources/mesh.h>
#include <resources/font.h>

#include "assetmanager.h"

#include <QDebug>
#include <QMap>
#include <QMenu>

#define COLOR       "Color"
#define COMPONENT   "Component"

Q_DECLARE_METATYPE(Vector4)

QVariant qVariant(Variant &v, const string &type) {
    switch(v.userType()) {
        case MetaType::BOOLEAN:
            return QVariant(v.toBool());
        case MetaType::INTEGER:
            return QVariant(v.toInt());
        case MetaType::FLOAT:
            return QVariant(v.toFloat());
        case MetaType::STRING:
            return QVariant(v.toString().c_str());
        case MetaType::VECTOR3:
            return QVariant::fromValue(v.toVector3());
        case MetaType::VECTOR4: {
            Vector4 value = v.toVector4();
            if(type == COLOR) {
                QColor r;
                r.setRgbF(value.x, value.y, value.z, value.w);
                return QVariant(r);
            }
            return QVariant::fromValue(value);
        }
        default: break;
    }
    Object *o   = *(reinterpret_cast<Object **>(v.data()));
    return QVariant::fromValue(Template(Engine::reference(o).c_str(), v.userType()));
}

Variant aVariant(QVariant &v, int type) {
    switch(type) {
        case MetaType::BOOLEAN: {
            return Variant(v.toBool());
        }
        case MetaType::INTEGER: {
            return Variant(v.toInt());
        }
        case MetaType::FLOAT: {
            return Variant(v.toFloat());
        }
        case MetaType::STRING: {
            if(v.canConvert<QFileInfo>()) {
                QFileInfo p  = v.value<QFileInfo>();
                return Variant(qUtf8Printable(p.absoluteFilePath()));
            }
            if(v.canConvert<Template>()) {
                Template p  = v.value<Template>();
                return Variant(qUtf8Printable(p.path));
            }
            return Variant(qUtf8Printable(v.toString()));
        }
        case MetaType::VECTOR3: {
            return Variant(v.value<Vector3>());
        }
        case MetaType::VECTOR4: {
            if(v.canConvert<QColor>()) {
                QColor c    = v.value<QColor>();
                Vector4 v(c.redF(), c.greenF(), c.blueF(), c.alphaF());
                return Variant(v);
            }
            return Variant(v.value<Vector4>());
        }
        default: {
            Template p  = v.value<Template>();
            if(!p.path.isEmpty()) {
                Object *m = Engine::loadResource<Object>(qPrintable(p.path));
                return Variant(type, &m);
            }
        } break;
    }
    return Variant();
}

NextObject::NextObject(Object *data, ObjectCtrl *ctrl, QObject *parent) :
        QObject(parent),
        m_pObject(data),
        m_pController(ctrl) {

    onUpdated();
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

QMenu *NextObject::menu(const QString &name) {
    QMenu *result   = nullptr;

    QStringList path(name.split(' ').first());
    Object *obj = findChild(path);

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

void NextObject::onUpdated() {
    foreach(QByteArray it, dynamicPropertyNames()) {
        setProperty(it, QVariant());
    }

    if(m_pObject) {
        buildObject(m_pObject);

        setObjectName(m_pObject->typeName().c_str());
    }
    emit updated();
}

void NextObject::onDeleteComponent() {
    emit deleteComponent(sender()->property(COMPONENT).toString());
}

void NextObject::buildObject(Object *object, const QString &path) {
    const MetaObject *meta = object->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        QString name    = (path.isEmpty() ? "" : path + "/") + property.name();
        Variant data    = property.read(object);

        if(data.userType() == MetaType::type<MaterialArray>()) {
            MaterialArray array = data.value<MaterialArray>();
            for(uint32_t i = 0; i < array.size(); i++) {
                Variant v = Variant::fromValue(array[i]->material());
                blockSignals(true);
                setProperty( qPrintable(name + "/Item" + QString::number(i)), qVariant(v, "") );
                blockSignals(false);
            }
        } else {
            blockSignals(true);
            setProperty( qPrintable(name), qVariant(data, property.type().name()) );
            blockSignals(false);
        }
    }
    for(Object *it : object->getChildren()) {
        Invalid *invalid = dynamic_cast<Invalid *>(it);
        if(invalid) {
            blockSignals(true);
            invalid->setName(tr("%1 (Invalid)").arg(invalid->typeName().c_str()).toStdString());
            setProperty( qPrintable((path.isEmpty() ? QString() : path + "/") + invalid->name().c_str() + QString("/")), QVariant(true) );
            blockSignals(false);
        } else if(dynamic_cast<Component *>(it)) {
            buildObject(it, (path.isEmpty() ? "" : path + "/") + QString::fromStdString(it->typeName()));
        }
    }
}

bool NextObject::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        QString name    = ev->propertyName();
        QVariant value  = property(qPrintable(name));
        if(value.isValid()) {
            QStringList list = name.split('/');
            if(m_pObject) {
                Object *o = findChild(list);
                Variant current = o->property(qPrintable(list.front()));
                Variant target;
                if(current.userType() == MetaType::type<MaterialArray>()) {
                    MaterialArray array = current.value<MaterialArray>();
                    uint32_t id = name.mid(name.indexOf(QRegExp("[0-9]"))).toInt();
                    if(id < array.size()) {
                        Material *m = aVariant(value, MetaType::type<Material *>()).value<Material *>();
                        if(m) {
                            array[id] = m->createInstance();
                        }
                    }
                    target  = Variant::fromValue(array);
                } else {
                    target  = aVariant(value, current.userType());
                }

                if(target.isValid() && current != target) {
                    if(m_pController) {
                        UndoManager::instance()->push(new UndoManager::PropertyObjects({m_pObject}, m_pController));
                    }

                    o->setProperty(qPrintable(list.front()), target);
                    onUpdated();

                    emit changed();
                    setChanged(o, list.front());
                }
            }
        }
    }
    return false;
}

Object *NextObject::findChild(QStringList &path) {
    Object *parent  = m_pObject;
    foreach(QString str, path) {
        for(Object *it : parent->getChildren()) {
            if(it->typeName() == str.toStdString()) {
                parent  = it;
                path.pop_front();
                break;
            }
        }
    }
    return parent;
}

void NextObject::setChanged(Object *object, const QString &property) {
    emit changed(object, property);
}
