#include "components/uiloader.h"

#include "components/actor.h"
#include "components/widget.h"
#include "components/recttransform.h"

#include "stylesheet.h"

#include "utils/stringutil.h"

#include <gizmos.h>
#include <pugixml.hpp>

namespace {
    const char *gUi("ui");
    const char *gName("name");
    const char *gStyle("style");
    const char *gClass("class");
};

void loadElementHelper(pugi::xml_node &node, Actor *actor) {
    string type = node.name();
    string name = node.attribute(gName).as_string();

    Actor *element = dynamic_cast<Actor *>(actor->find(name));
    if(element == nullptr) {
        element = Engine::composeActor(type, name, actor);
    }

    Widget *widget = dynamic_cast<Widget *>(element->component(type));
    if(widget) {
        const MetaObject *meta = widget->metaObject();
        for(auto it : node.attributes()) {
            int32_t index = meta->indexOfProperty(it.name());
            if(index > -1) {
                MetaProperty property = meta->property(index);
                Variant current = widget->property(property.name());

                switch(current.type()) {
                case MetaType::BOOLEAN: widget->setProperty(property.name(), it.as_bool()); break;
                case MetaType::INTEGER: widget->setProperty(property.name(), it.as_int()); break;
                case MetaType::FLOAT: widget->setProperty(property.name(), it.as_float()); break;
                case MetaType::STRING: widget->setProperty(property.name(), it.as_string()); break;
                default: break;
                }
            }
        }

        string classes = node.attribute(gClass).as_string();
        if(!classes.empty()) {
            for(auto &it : StringUtil::split(classes, ' ')) {
                widget->addClass(it);
            }
        }

        string style = node.attribute(gStyle).as_string();
        if(!style.empty()) {
            StyleSheet::resolveInline(widget, style);
        }
    }

    for(pugi::xml_node it : node.children()) {
        loadElementHelper(it, element);
    }
}

UiLoader::UiLoader() :
        m_document(nullptr),
        m_styleSheet(nullptr) {

}

void UiLoader::fromBuffer(const string &buffer) {
    cleanHierarchy(this);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.c_str(), buffer.size());

    if(result) {
        for(pugi::xml_node node : doc.child(gUi).children()) {
            string type = node.name();
            if(type == gStyle) {
                m_documentStyle = node.text().as_string();

                if(m_styleSheet) {
                    m_styleSheet->addRawData(m_documentStyle);
                }
            } else {
                loadElementHelper(node, actor());
            }
        }

        applyStyle();
    }
}

string UiLoader::documentStyle() const {
    return m_documentStyle;
}

UiDocument *UiLoader::document() const {
    return m_document;
}

void UiLoader::setUiDocument(UiDocument *document) {
    if(m_document != document) {
        m_document = document;

        fromBuffer(m_document->data());
    }
}
/*!
    Returns a style sheet assigned to the hierarhy of widgets.
*/
StyleSheet *UiLoader::styleSheet() const {
    return m_styleSheet;
}
/*!
    Sets a \a style sheet to the hierarhy of widgets.
*/
void UiLoader::setStyleSheet(StyleSheet *style) {
    if(m_styleSheet != style) {
        m_styleSheet = style;

        if(m_styleSheet) {
            m_styleSheet->addRawData(m_documentStyle);

            resolveStyleSheet(this);
        }

        applyStyle();
    }
}
/*!
    \internal
*/
void UiLoader::resolveStyleSheet(Widget *widget) {
    for(auto it : widget->childWidgets()) {
        m_styleSheet->resolve(it);
        resolveStyleSheet(widget);
    }
}
/*!
    \internal
*/
void UiLoader::cleanHierarchy(Widget *widget) {
    for(auto it : widget->childWidgets()) {
        delete it->actor();
    }
}
/*!
    \internal
*/
void UiLoader::drawGizmos() {
    AABBox box = m_transform->bound();
    Gizmos::drawRectangle(box.center, Vector2(box.extent.x * 2.0f,
                                              box.extent.y * 2.0f), Vector4(0.5f, 0.5f, 1.0f, 1.0f));
}
