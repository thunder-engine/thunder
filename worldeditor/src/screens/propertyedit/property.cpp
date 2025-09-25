#include "property.h"

#include <system.h>
#include <invalid.h>
#include <world.h>
#include <editor/propertyedit.h>

#include <QFileInfo>
#include <QLocale>

#include "custom/component/actions.h"
#include "custom/objectselect/objectselect.h"
#include "custom/nextenum/nextenumedit.h"
#include "custom/vector4/vector4edit.h"

namespace {
    const char *gEditorTag("editor=");
    const char *gEnumTag("enum=");
    const char *gTypeTag("type=");

    const char *gAlignment("Alignment");
    const char *gAsset("Asset");
    const char *gAxises("Axises");
    const char *gColor("Color");
    const char *gResource("Resource");
    const char *gPath("Path");
    const char *gLocale("Locale");
    const char *gReadOnlyTag("ReadOnly");

    const char *gEnabled("enabled");
}

Property::Property(const TString &name, Property *parent, bool root) :
        QObject(parent),
        m_nextObject(nullptr),
        m_editor(nullptr),
        m_root(root),
        m_checkable(false),
        m_readOnly(m_root) {

    StringList list = name.split('/');

    m_name = list.back();
    setObjectName(name.data());
    m_root = (root) ? (list.size() == 1) : false;
}

void Property::setPropertyObject(Object *propertyObject) {
    m_nextObject = propertyObject;

    Invalid *invalid = dynamic_cast<Invalid *>(m_nextObject);
    if(invalid) {
        m_name += " (Invalid)";
    }

    m_readOnly = hasTag(m_hints, gReadOnlyTag);

    if(m_root) {
        const MetaObject *meta = m_nextObject->metaObject();

        int index = meta->indexOfProperty(gEnabled);
        m_checkable = (index > -1);
    }
}

TString Property::editorHints() const {
    return m_hints;
}

void Property::setEditorHints(const TString &hints) {
    m_hints = hints;
}

QVariant Property::value(int role) const {
    if(role == Qt::DisplayRole) {
        PropertyEdit *editor = dynamic_cast<PropertyEdit *>(m_editor);
        if(editor) {
            return editor->data();
        }
    } else if(role != Qt::DecorationRole) {
        if(m_nextObject) {
            const MetaObject *meta = m_nextObject->metaObject();
            int index = meta->indexOfProperty(m_name.data());

            if(index > -1) {
                const MetaProperty property(meta->property(index));

                return qVariant(property.read(m_nextObject), m_hints, property.type().name(), m_nextObject);
            } else { // Dynamic property
                Variant value = m_nextObject->property(qPrintable(objectName()));
                TString typeName;
                if(value.isValid()) {
                    typeName = MetaType::name(value.userType());
                }

                return qVariant(value, m_hints, typeName, m_nextObject);
            }
        }
    }
    return QVariant();
}

void Property::setValue(const QVariant &value) {
    if(m_nextObject) {
        Variant current(m_nextObject->property(qPrintable(objectName())));

        const MetaObject *meta = m_nextObject->metaObject();
        int index = meta->indexOfProperty(m_name.data());

        Variant target;
        if(index > -1) {
            MetaProperty property(meta->property(index));

            TString typeName = MetaType::name(current.userType());
            if(property.isValid()) {
                typeName = property.type().name();
            }

            target = aVariant(value, current.userType(), typeName);
        } else {
            target = aVariant(value, current.userType(), TString());
        }

        if(target != current) {
            AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_nextObject);
            if(settings) {
                m_nextObject->setProperty(m_name.data(), target);
            } else {
                emit propertyChanged({m_nextObject}, objectName().toStdString(), target);
            }
        }

    }
}

TString Property::name() const {
    if(!m_name.isEmpty()) {
        return m_name;
    }

    return objectName().toStdString();
}

void Property::setName(const TString &value) {
    m_name = value;
}

QWidget *Property::editor() const {
    return m_editor;
}

bool Property::isRoot() const {
    return m_root;
}

bool Property::isReadOnly() const {
    return m_readOnly;
}

QWidget *Property::getEditor(QWidget *parent) const {
    if(m_editor) {
        if(parent) {
            m_editor->setParent(parent);
        }
        return m_editor;
    }
    return createEditor(parent);
}

QWidget *Property::createEditor(QWidget *parent) const {
    int32_t type = 0;

    if(m_nextObject) {
        QVariant data = value(Qt::EditRole);
        if(data.isValid()) {
            type = data.userType();
        }
    }

    PropertyEdit *editor = PropertyEdit::constructEditor(type, parent, objectName().toStdString());
    if(editor == nullptr && m_root) {
        editor = new Actions(parent);
    }

    if(editor) {
        if(m_nextObject) {
            editor->setObject(m_nextObject, m_name);
        }

        editor->setDisabled(isReadOnly());
        editor->setEditorHint(m_hints);

        connect(editor, &PropertyEdit::editFinished, this, &Property::onDataChanged);
        connect(editor, &PropertyEdit::dataChanged, this, &Property::onDataChanged);
        connect(editor, &Actions::destroyed, this, &Property::onEditorDestoyed);
        m_editor = editor;
    }

    return m_editor;
}

QSize Property::sizeHint(const QSize &size) const {
    return QSize(size.width(), m_editor ? m_editor->height() : size.height());
}

QVariant Property::editorData(QWidget *editor) {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(editor);
    if(e) {
        return e->data();
    }
    return QVariant();
}

bool Property::setEditorData(QWidget *editor, const QVariant &data) {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data);
        e->blockSignals(false);
        return true;
    }
    return false;
}

bool Property::isCheckable() const {
    return m_checkable;
}

bool Property::isChecked() const {
    if(m_root) {
        Actions *actions = static_cast<Actions *>(m_editor);
        if(actions) {
            return static_cast<Actions *>(m_editor)->isChecked();
        }
    }

    return false;
}

void Property::setChecked(bool value) {
    if(m_root) {
        Actions *actions = static_cast<Actions *>(m_editor);
        if(actions) {
            actions->onDataChanged(value);
        }
    }
}

void Property::onDataChanged() {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(m_editor);
    if(e) {
        setValue(e->data());
    }
}

void Property::onEditorDestoyed() {
    m_editor = nullptr;
}

QVariant Property::qVariant(const Variant &value, const TString &hints, const TString &typeName, Object *object) {
    if(!value.isValid()) {
        return QVariant();
    }

    TString editor(propertyTag(hints, gEditorTag));

    switch(value.userType()) {
        case MetaType::BOOLEAN: {
            return QVariant(value.toBool());
        }
        case MetaType::INTEGER: {
            TString enumProperty = propertyTag(hints, gEnumTag);

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
            TString str = value.toString();

            if(editor == gPath) {
                return QVariant::fromValue(QFileInfo(str.data()));
            } else if(editor == gLocale) {
                return QVariant::fromValue(QLocale(str.data()));
            } else if(editor == gAsset) {
                return QVariant::fromValue(Template(str, propertyTag(hints, gTypeTag)));
            }
            return QVariant(str.data());
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
    TString typeNameTrimmed = typeName;
    trimmType(typeNameTrimmed, isArray);

    if(isArray) {
        QVariantList result;
        for(auto &it : *(reinterpret_cast<VariantList *>(value.data()))) {
            result << qVariant(it, hints, typeNameTrimmed, object);
        }

        return result;
    }

    return qObjectVariant(value, typeNameTrimmed, editor, object);
}

QVariant Property::qObjectVariant(const Variant &value, const TString &typeName, const TString &editor, Object *object) {
    auto factory = System::metaFactory(typeName);
    if(factory) {
        Object *objectValue = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
        if(factory->first->canCastTo(gResource) || (editor == gAsset)) {
            return QVariant::fromValue(Template(Engine::reference(objectValue), MetaType::name(value.userType())));
        } else {
            Scene *scene = nullptr;
            Actor *actor = dynamic_cast<Actor *>(objectValue);
            Component *component = dynamic_cast<Component *>(objectValue);

            if(actor) {
                scene = actor->scene();
            } else if(component) {
                scene = component->scene();
            }

            if(scene == nullptr) {
                Actor *nextActor = dynamic_cast<Actor *>(object);
                if(nextActor) {
                    scene = nextActor->scene();
                } else {
                    Component *nextActorComp = dynamic_cast<Component *>(object);
                    if(nextActorComp) {
                        scene = nextActorComp->scene();
                    }
                }
            }

            ObjectData cmp;
            cmp.type = typeName;
            cmp.component = component;
            cmp.actor = actor;
            cmp.scene = scene;

            return QVariant::fromValue(cmp);
        }
    }

    return QVariant();
}

Variant Property::aVariant(const QVariant &value, const uint32_t type, const TString &typeName) {
    if(type == 0) {
        return Variant();
    }
    TString editor(propertyTag(m_hints, gEditorTag));

    switch(type) {
        case MetaType::BOOLEAN: {
            return Variant(value.toBool());
        }
        case MetaType::INTEGER: {
            TString enumProperty = propertyTag(m_hints, gEnumTag);
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
                return Variant(p.absoluteFilePath().toStdString());
            } else if(value.canConvert<Template>()) {
                Template p = value.value<Template>();
                return Variant(p.path);
            } else if(value.canConvert<QLocale>()) {
                return Variant(value.value<QLocale>().bcp47Name().toStdString());
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
    TString trimmedTypeName = typeName;
    trimmType(trimmedTypeName, isArray);

    if(isArray) {
        VariantList result;

        uint32_t userType = MetaType::type(trimmedTypeName.data());
        if(userType > MetaType::USERTYPE) {
            userType++;
        }

        for(auto &it : value.toList()) {
             result.push_back(aVariant(it, userType, trimmedTypeName));
        }

        return result;
    }

    return aObjectVariant(value, type, typeName);
}

Variant Property::aObjectVariant(const QVariant &value, uint32_t type, const TString &typeName) {
    auto factory = System::metaFactory(typeName);
    if(factory) {
        if(factory->first->canCastTo(gResource)) {
            if(value.isValid()) {
                Template p = value.value<Template>();
                if(!p.path.isEmpty()) {
                    Object *m = Engine::loadResource<Object>(p.path);
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

TString Property::propertyTag(const TString &hints, const TString &tag) {
    StringList list(hints.split(','));
    for(TString it : list) {
        int index = it.indexOf(tag);
        if(index > -1) {
            return it.remove(tag);
        }
    }
    return TString();
}

bool Property::hasTag(const TString &hints, const TString &tag) {
    StringList list(hints.split(','));
    for(TString it : list) {
        int index = it.indexOf(tag);
        if(index > -1) {
            return true;
        }
    }
    return false;
}

void Property::trimmType(TString &type, bool &isArray) {
    if(type.back() == '*') {
        type.removeLast();
        while(type.back() == ' ') {
            type.removeLast();
        }
    } else if(type.back() == ']') {
        type.removeLast();
        while(type.back() == ' ') {
            type.removeLast();
        }
        if(type.back() == '[') {
            type.removeLast();
            isArray = true;
        }
    }
}
