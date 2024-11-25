#include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>

#include <object.h>
#include <invalid.h>

#include <engine.h>
#include <components/scene.h>
#include <components/actor.h>

#include "custom/array/arrayedit.h"
#include "custom/alignment/alignmentedit.h"
#include "custom/axises/axisesedit.h"
#include "custom/color/coloredit.h"
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
    const char *gAsset("Asset");
}

NextObject::NextObject(QObject *parent) :
        QObject(parent),
        m_object(nullptr) {

    setProperty("_next", true);

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

    for(auto &it : object->dynamicPropertyNames()) {
        Variant value(object->property(it.c_str()));
        MetaProperty property({});

        setProperty(qPrintable(p + it.c_str()), qVariant(value, property, object));
    }

    blockSignals(false);

    for(Object *it : object->getChildren()) {
        class Invalid *invalid = dynamic_cast<class Invalid *>(it);
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

            Variant target;
            if(index > -1) {
                MetaProperty property(meta->property(index));
                target = aVariant(value, current, property);
            } else {
                MetaProperty property({});
                target = aVariant(value, current, property);
            }

            if(target.isValid() && current != target) {
                emit propertyChanged({o}, propertyName, target);
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

inline void trimmType(std::string &type, bool &isArray) {
    if(type.back() == '*') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
    } else if(type.back() == ']') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
        if(type.back() == '[') {
            type.pop_back();
            isArray = true;
        }
    }
}

QVariant NextObject::qVariant(const Variant &value, const MetaProperty &property, Object *object) {
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

    bool isArray = false;
    std::string typeName = property.type().name();
    trimmType(typeName, isArray);

    if(isArray) {
        QVariantList result;
        for(auto &it : *(reinterpret_cast<VariantList *>(value.data()))) {
            result << qObjectVariant(it, typeName, editor);
        }

        return result;
    }

    return qObjectVariant(value, typeName, editor);
}

QVariant NextObject::qObjectVariant(const Variant &value, const std::string &typeName, const QString &editor) {
    auto factory = System::metaFactory(typeName);
    if(factory) {
        Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
        if(factory->first->canCastTo(gResource) || (editor == gAsset)) {
            return QVariant::fromValue(Template(Engine::reference(object).c_str(), value.userType()));
        } else {
            Scene *scene = nullptr;
            Actor *actor = dynamic_cast<Actor *>(object);
            Component *component = dynamic_cast<Component *>(object);

            if(actor) {
                scene = actor->scene();
            } else if(component) {
                scene = component->scene();
            }

            if(scene == nullptr) {
                Actor *actor = dynamic_cast<Actor *>(m_object);
                if(actor) {
                    scene = actor->scene();
                }
            }

            ObjectData cmp;
            cmp.type = typeName;
            cmp.component = dynamic_cast<Component *>(object);
            cmp.actor = dynamic_cast<Actor *>(object);
            cmp.scene = scene;

            return QVariant::fromValue(cmp);
        }
    }

    return QVariant();
}

Variant NextObject::aVariant(const QVariant &value, const Variant &current, const MetaProperty &property) {
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

    bool isArray = false;
    std::string typeName = property.type().name();
    trimmType(typeName, isArray);

    if(isArray) {
        VariantList result;

        for(auto &it : value.toList()) {
            uint32_t usertType = current.userType();
            if(usertType == MetaType::VARIANTLIST) {
                VariantList &list = *(reinterpret_cast<VariantList *>(current.data()));
                if(!list.empty()) {
                    usertType = list.front().userType();
                } else {
                    usertType = MetaType::type(typeName.c_str()) + 1;
                }
            }
            result.push_back(aObjectVariant(it, usertType, typeName));
        }

        return result;
    }

    return aObjectVariant(value, current.userType(), typeName);
}

Variant NextObject::aObjectVariant(const QVariant &value, uint32_t type, const std::string &typeName) {
    auto factory = System::metaFactory(typeName);
    if(factory) {
        if(factory->first->canCastTo(gResource)) {
            if(value.isValid()) {
                Template p = value.value<Template>();
                if(!p.path.isEmpty()) {
                    Object *m = Engine::loadResource<Object>(qPrintable(p.path));
                    return Variant(type, &m);
                }
            }
            return Variant(type, nullptr);
        } else {
            if(value.isValid()) {
                ObjectData c(value.value<ObjectData>());
                if(c.component) {
                    return Variant(type, &c.component);
                }
                if(c.actor) {
                    return Variant(type, &c.actor);
                }
            }
            return Variant(type, nullptr);
        }
    }

    return Variant();
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

    return result;
}
