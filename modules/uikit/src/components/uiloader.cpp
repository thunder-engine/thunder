#include "components/uiloader.h"

#include "components/actor.h"
#include "components/widget.h"
#include "components/recttransform.h"

#include "stylesheet.h"

#include <gizmos.h>
#include <pugixml.hpp>

namespace {
    const char *gUi("ui");
    const char *gName("name");
    const char *gStyle("style");
    const char *gClass("class");
};

void loadElementHelper(pugi::xml_node &node, Actor *actor) {
    std::string type = node.name();
    std::string name = node.attribute(gName).as_string();

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

        String classes = node.attribute(gClass).as_string();
        if(!classes.isEmpty()) {
            for(auto &it : classes.split(' ')) {
                widget->addClass(it);
            }
        }

        String style = node.attribute(gStyle).as_string();
        if(!style.isEmpty()) {
            StyleSheet::resolveInline(widget, style);
        }
    }

    for(pugi::xml_node it : node.children()) {
        loadElementHelper(it, element);
    }
}
/*!
    \class UiLoader
    \brief The UiLoader class, is responsible for loading and managing UI data (likely XML-based) and applying a style sheet to a hierarchy of widgets.
    \inmodule Gui

    The UiLoader class is tasked with loading user interface (UI) data from a buffer (likely an XML or similar format), managing the UI document, applying styles to the UI elements, and maintaining a hierarchy of widgets.
    This class appears to be part of a larger UI framework that supports widget-based layouts.
*/

UiLoader::UiLoader() :
        m_document(nullptr),
        m_styleSheet(nullptr) {

}
/*!
    This function loads the UI data from an XML \a buffer (likely containing UI element definitions and style information).
*/
void UiLoader::fromBuffer(const String &buffer) {
    cleanHierarchy(this);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

    if(result) {
        for(pugi::xml_node node : doc.child(gUi).children()) {
            std::string type = node.name();
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
/*!
    Returns the raw document style (as a string), which was parsed from the UI document.
*/
String UiLoader::documentStyle() const {
    return m_documentStyle;
}
/*!
    Returns the UiDocument associated with this UiLoader, which contains the structure of the loaded UI.
*/
UiDocument *UiLoader::document() const {
    return m_document;
}
/*!
    Sets the UI document to the provided \a document pointer and reloads the UI from the document's data buffer by calling fromBuffer().
*/
void UiLoader::setDocument(UiDocument *document) {
    if(m_document != document) {
        m_document = document;

        if(m_document) {
            fromBuffer(m_document->data());
        } else {
            cleanHierarchy(this);
        }
    }
}
/*!
    Returns the style sheet assigned to the hierarchy of widgets.
    This contains the visual styles (like colors, margins, fonts, etc.) that should be applied to the widgets.
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
        if(widget != this) {
            resolveStyleSheet(widget);
        }
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
