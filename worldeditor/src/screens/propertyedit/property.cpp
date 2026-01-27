#include "property.h"

#include <system.h>
#include <invalid.h>
#include <editor/propertyedit.h>

#include "custom/component/actions.h"
#include "custom/objectselect/objectselect.h"
#include "custom/nextenum/nextenumedit.h"

namespace {
    const char *gEditorTag("editor=");
    const char *gEnumTag("enum=");
    const char *gTypeTag("type=");

    const char *gAsset("Asset");
    const char *gComponent("Component");

    const char *gAlignment("Alignment");
    const char *gAxises("Axises");
    const char *gColor("Color");
    const char *gPath("Path");
    const char *gLocale("Locale");

    const char *gResource("Resource");

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

    if(m_nextObject) {
        const MetaObject *meta = m_nextObject->metaObject();

        if(m_root) {
            int index = meta->indexOfProperty(gEnabled);
            m_checkable = (index > -1);
        }

        int index = meta->indexOfProperty(m_name.data());
        if(index > -1) {
            const MetaProperty property(meta->property(index));
            m_typeNameTrimmed = property.type().name();
        } else { // Dynamic property
            Variant value = m_nextObject->property(qPrintable(objectName()));
            if(value.isValid()) {
                m_typeNameTrimmed = MetaType::name(value.userType());
            }
        }

        if(!m_typeNameTrimmed.isEmpty()) {
            bool isArray;
            trimmType(m_typeNameTrimmed, isArray);
        }
    }
}

TString Property::editorHints() const {
    return m_hints;
}

void Property::setEditorHints(const TString &hints) {
    m_hints = hints;
}

Variant Property::value() const {
    if(m_nextObject) {
        return m_nextObject->property(qPrintable(objectName()));
    }

    return Variant();
}

void Property::setValue(const Variant &value) {
    if(m_nextObject) {
        Variant current(m_nextObject->property(qPrintable(objectName())));

        if(value != current) {
            AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_nextObject);
            if(settings) {
                m_nextObject->setProperty(m_name.data(), value);
            } else {
                emit propertyChanged({m_nextObject}, objectName().toStdString(), value);
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

PropertyEdit *Property::editor() const {
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

PropertyEdit *Property::createEditor(QWidget *parent) const {
    int32_t type = 0;

    if(m_nextObject) {
        Variant data = value();
        if(data.isValid()) {
            type = data.userType();
        }
    }

    TString editorTag = editorName(m_hints, m_typeNameTrimmed);

    PropertyEdit *editor = PropertyEdit::constructEditor(type, parent, editorTag);
    if(editor == nullptr && m_root) {
        editor = new Actions(parent);
    }

    if(editor) {
        TString enumProperty = propertyTag(m_hints, gEnumTag);
        if(!enumProperty.isEmpty()) {
            NextEnumEdit *edit = dynamic_cast<NextEnumEdit *>(editor);
            if(edit) {
                edit->setEnumData(enumProperty, m_nextObject);
            }
        } else {
            ObjectSelect *edit = dynamic_cast<ObjectSelect *>(editor);
            if(edit) {
                TString type(propertyTag(m_hints, gTypeTag));
                if(type.isEmpty()) {
                    type = m_typeNameTrimmed;
                }

                edit->setType(type);
            }
        }

        if(m_nextObject) {
            editor->setObject(m_nextObject, qPrintable(objectName()));
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

void Property::updateEditor() {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(m_editor);
    if(e) {
        e->blockSignals(true);
        e->setData(value());
        e->blockSignals(false);
    }
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

TString Property::editorName(const TString &hints, const TString &typeName) {
    TString enumProperty = propertyTag(hints, gEnumTag);
    TString editorTag = propertyTag(hints, gEditorTag);
    if(editorTag.isEmpty()) {
        if(!enumProperty.isEmpty()) {
            editorTag = "Enum";
        }

        auto factory = System::metaFactory(typeName);
        if(factory) {
            if(factory->first->canCastTo(gResource)) {
                editorTag = gAsset;
            } else {
                editorTag = gComponent;
            }
        }
    }

    return editorTag;
}

TString Property::propertyTag(const TString &hints, const TString &tag) {
    StringList list(hints.split(','));
    for(TString &it : list) {
        int index = it.indexOf(tag);
        if(index > -1) {
            return it.remove(tag);
        }
    }
    return TString();
}

bool Property::hasTag(const TString &hints, const TString &tag) {
    StringList list(hints.split(','));
    for(const TString &it : list) {
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
