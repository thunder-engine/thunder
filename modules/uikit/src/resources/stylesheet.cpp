#include "stylesheet.h"

#include <log.h>

#include <regex>

#include "components/widget.h"
#include "utils/selector.h"
#include "utils/cssparser.h"

namespace {
    const char *gData("Data");
}

/*!
    \class StyleSheet
    \brief The StyleSheet class that appears to handle the parsing, storage, and application of CSS styles.
    \inmodule Gui

    The StyleSheet class is responsible for handling CSS style rules, which can be applied to Widget objects.
    It includes functionality for storing, loading, saving, and resolving styles, along with utility methods to convert CSS color and length values.
*/

StyleSheet::StyleSheet() :
        m_parser(new CSSParser()) {

}

/*!
    Returns content as a string.
*/
TString StyleSheet::data() const {
    return m_data;
}
/*!
    Sets a new content \a data.
*/
void StyleSheet::setData(const TString &data) {
    m_data = data;

    addRawData(m_data);
}

bool StyleSheet::addRawData(const TString &data) {
    return reinterpret_cast<CSSParser *>(m_parser)->parseByString(data);
}
/*!
    \internal
*/
void StyleSheet::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        setData((*it).second.toString());
    }
}
/*!
    \internal
*/
VariantMap StyleSheet::saveUserData() const {
    VariantMap result;
    result[gData] = m_data;
    return result;
}

/*!
    Resolves the styles for a given \a widget based on the parsed CSS rules.
    It iterates through the CSS selectors and applies matching rules to the widget.
*/
void StyleSheet::resolve(Widget *widget) {
    CSSParser *parser = reinterpret_cast<CSSParser *>(m_parser);

    for(auto it : parser->selectors()) {
        if(it->isMeet(widget)) {
            widget->addStyleRules(it->ruleDataMap(), it->weight());
        }
    }
}
/*!
    Resolves inline styles provided as a string (e.g., from the \a style attribute in XML).
    The method splits the inline styles into key-value pairs and applies them to the \a widget.
*/
void StyleSheet::resolveInline(Widget *widget, const TString &style) {
    TString inlineStyle = style;
    if(!inlineStyle.isEmpty()) {
        std::map<TString, TString> result;

        inlineStyle = inlineStyle.trimmed();
        auto keyValues = Selector::splitButSkipBrackets(inlineStyle.toStdString(), ';');

        for(const auto &pair : keyValues) {
            auto keyAndValue = Selector::splitButSkipBrackets(pair.toStdString(), ':');

            if(keyAndValue.size() < 2) {
                continue;
            }
            result[keyAndValue[0].trimmed()] = keyAndValue[1].trimmed();
        }

        widget->addStyleRules(result, 1000);
    }
}
/*!
    Directly sets a style property for a \a widget.
    It adds the specified \a key and \a value pair to the widget's style rules.
*/
void StyleSheet::setStyleProperty(Widget *widget, const TString &key, const TString &value) {
    if(widget) {
        widget->addStyleRules({{key, value}}, 1000);
    }
}
/*!
    Converts a CSS color \a value (e.g., named colors like "blue" or hexadecimal colors like "#ff0000") to a Vector4 representing RGBA values.
    It handles both named colors and hex codes (including short hex formats like #FFF).
*/
Vector4 StyleSheet::toColor(const TString &value) {
    static std::map<TString, TString> colors = {
        {"aliceblue",       "#fff0f8ff"}, {"antiquewhite",    "#fffaebd7"}, {"aqua",            "#ff00ffff"}, {"aquamarine",      "#ff7fffd4"},
        {"azure",           "#fff0ffff"}, {"beige",           "#fff5f5dc"}, {"bisque",          "#ffffe4c4"}, {"black",           "#ff000000"},
        {"blanchedalmond",  "#ffffebcd"}, {"blue",            "#ff0000ff"}, {"blueviolet",      "#ff8a2be2"}, {"brown",           "#ffa52a2a"},
        {"burlywood",       "#ffdeb887"}, {"cadetblue",       "#ff5f9ea0"}, {"chartreuse",      "#ff7fff00"}, {"chocolate",       "#ffd2691e"},
        {"coral",           "#ffff7f50"}, {"cornflowerblue",  "#ff6495ed"}, {"cornsilk",        "#fffff8dc"}, {"crimson",         "#ffdc143c"},
        {"cyan",            "#ff00ffff"}, {"darkblue",        "#ff00008b"}, {"darkcyan",        "#ff008b8b"}, {"darkgoldenrod",   "#ffb8860b"},
        {"darkgray",        "#ffa9a9a9"}, {"darkgreen",       "#ff006400"}, {"darkgrey",        "#ffa9a9a9"}, {"darkkhaki",       "#ffbdb76b"},
        {"darkmagenta",     "#ff8b008b"}, {"darkolivegreen",  "#ff556b2f"}, {"darkorange",      "#ffff8c00"}, {"darkorchid",      "#ff9932cc"},
        {"darkred",         "#ff8b0000"}, {"darksalmon",      "#ffe9967a"}, {"darkseagreen",    "#ff8fbc8f"}, {"darkslateblue",   "#ff483d8b"},
        {"darkslategray",   "#ff2f4f4f"}, {"darkslategrey",   "#ff2f4f4f"}, {"darkturquoise",   "#ff00ced1"}, {"darkviolet",      "#ff9400d3"},
        {"deeppink",        "#ffff1493"}, {"deepskyblue",     "#ff00bfff"}, {"dimgray",         "#ff696969"}, {"dimgrey",         "#ff696969"},
        {"dodgerblue",      "#ff1e90ff"}, {"firebrick",       "#ffb22222"}, {"floralwhite",     "#fffffaf0"}, {"forestgreen",     "#ff228b22"},
        {"gainsboro",       "#ffdcdcdc"}, {"ghostwhite",      "#fff8f8ff"}, {"gold",            "#ffffd700"}, {"gray",            "#ff808080"},
        {"green",           "#ff008000"}, {"goldenrod",       "#ffdaa520"}, {"greenyellow",     "#ffadff2f"}, {"grey",            "#ff808080"},
        {"honeydew",        "#fff0fff0"}, {"hotpink",         "#ffff69b4"}, {"indianred",       "#ffcd5c5c"}, {"indigo",          "#ff4b0082"},
        {"ivory",           "#fffffff0"}, {"khaki",           "#fff0e68c"}, {"lavender",        "#ffe6e6fa"}, {"lavenderblush",   "#fffff0f5"},
        {"lawngreen",       "#ff7cfc00"}, {"lemonchiffon",    "#fffffacd"}, {"lightblue",       "#ffadd8e6"}, {"lightcoral",      "#fff08080"},
        {"lightcyan",       "#ffe0ffff"}, {"lightgray",       "#ffd3d3d3"}, {"lightgreen",      "#ff90ee90"}, {"lightgrey",       "#ffd3d3d3"},
        {"lightpink",       "#ffffb6c1"}, {"lightsalmon",     "#ffffa07a"}, {"lightseagreen",   "#ff20b2aa"}, {"lightskyblue",    "#ff87cefa"},
        {"lightslategray",  "#ff778899"}, {"lightslategrey",  "#ff778899"}, {"lightsteelblue",  "#ffb0c4de"}, {"lightyellow",     "#ffffffe0"},
        {"lime",            "#ff00ff00"}, {"limegreen",       "#ff32cd32"}, {"linen",           "#fffaf0e6"}, {"magenta",         "#ffff00ff"},
        {"maroon",          "#ff800000"}, {"mediumaquamarine","#ff66cdaa"}, {"mediumblue",      "#ff0000cd"}, {"mediumorchid",    "#ffba55d3"},
        {"mediumpurple",    "#ff9370db"}, {"mediumseagreen",  "#ff3cb371"}, {"mediumslateblue", "#ff7b68ee"}, {"mediumturquoise", "#ff48d1cc"},
        {"mediumvioletred", "#ffc71585"}, {"midnightblue",    "#ff191970"}, {"mintcream",       "#fff5fffa"}, {"mistyrose",       "#ffffe4e1"},
        {"moccasin",        "#ffffe4b5"}, {"navajowhite",     "#ffffdead"}, {"navy",            "#ff000080"}, {"oldlace",         "#fffdf5e6"},
        {"olive",           "#ff808000"}, {"olivedrab",       "#ff6b8e23"}, {"orange",          "#ffffa500"}, {"orangered",       "#ffff4500"},
        {"orchid",          "#ffda70d6"}, {"palegoldenrod",   "#ffeee8aa"}, {"palegreen",       "#ff98fb98"}, {"paleturquoise",   "#ffafeeee"},
        {"palevioletred",   "#ffdb7093"}, {"papayawhip",      "#ffffefd5"}, {"peachpuff",       "#ffffdab9"}, {"peru",            "#ffcd853f"},
        {"pink",            "#ffffc0cb"}, {"plum",            "#ffdda0dd"}, {"powderblue",      "#ffb0e0e6"}, {"purple",          "#ff800080"},
        {"rebeccapurple",   "#ff663399"}, {"red",             "#ffff0000"}, {"rosybrown",       "#ffbc8f8f"}, {"royalblue",       "#ff4169e1"},
        {"saddlebrown",     "#ff8b4513"}, {"salmon",          "#fffa8072"}, {"sandybrown",      "#fff4a460"}, {"seagreen",        "#ff2e8b57"},
        {"seashell",        "#fffff5ee"}, {"sienna",          "#ffa0522d"}, {"silver",          "#ffc0c0c0"}, {"skyblue",         "#ff87ceeb"},
        {"slateblue",       "#ff6a5acd"}, {"slategray",       "#ff708090"}, {"slategrey",       "#ff708090"}, {"snow",            "#fffffafa"},
        {"springgreen",     "#ff00ff7f"}, {"steelblue",       "#ff4682b4"}, {"tan",             "#ffd2b48c"}, {"teal",            "#ff008080"},
        {"thistle",         "#ffd8bfd8"}, {"tomato",          "#ffff6347"}, {"transparent",     "#00000000"}, {"turquoise",       "#ff40e0d0"},
        {"violet",          "#ffee82ee"}, {"wheat",           "#fff5deb3"}, {"white",           "#ffffffff"}, {"whitesmoke",      "#fff5f5f5"},
        {"yellow",          "#ffffff00"}, {"yellowgreen",     "#ff9acd32"},
    };

    TString str = value;
    auto it = colors.find(str);
    if(it != colors.end()) {
        str = it->second;
    }

    Vector4 result(0.0f, 0.0f, 0.0f, 1.0f);

    if(str.front() == '#') {
        std::string s = str.toStdString();
        uint32_t rgba = std::stoul(&s[1], nullptr, 16);

        uint32_t size = str.size();
        switch(size) {
            case 4: // #RGB
            case 5: { // #ARGB
                uint8_t p1 = rgba;
                uint8_t p0 = rgba >> 8;

                result.x = float((p0 & 0x0f) | ((p0 & 0x0f) << 4)) / 255.0f;
                result.y = float((p1 >> 4) | ((p1 >> 4) << 4)) / 255.0f;
                result.z = float((p1 & 0x0f) | ((p1 & 0x0f) << 4)) / 255.0f;
                if(size == 5) {
                    result.w = float((p0 >> 4) | ((p0 >> 4) << 4)) / 255.0f;
                }
            } break;
            case 7: // #RRGGBB
            case 9: { // #FFRRGGBB
                result.z = float(uint8_t(rgba)) / 255.0f;
                result.y = float(uint8_t(rgba >> 8)) / 255.0f;
                result.x = float(uint8_t(rgba >> 16)) / 255.0f;
                if(size == 9) {
                    result.w = float(uint8_t(rgba >> 24)) / 255.0f;
                }
            } break;
            default: break;
        }
    } else if(str.front() == 'r') {
        std::smatch match;
        std::regex_search(value.toStdString(), match, std::regex("([0-9]*[.]?[0-9]+),*[ ]*([0-9]*[.]?[0-9]+)?,*[ ]*([0-9]*[.]?[0-9]+)?,*[ ]*([0-9]*[.]?[0-9]+)?"));

        for(int i = 1; i < match.size(); i++) {
            std::string sub = match[i];
            if(sub.empty() || i == 4) {
                break;
            }
            result[i-1] = stof(match[i]);
        }

        result.x /= 255.0f;
        result.y /= 255.0f;
        result.z /= 255.0f;
    }

    return result;
}
/*!
    Converts a length \a value (e.g., "10px" or "50%") into a numeric value and returns whether the value is in \a pixels or percentages.
*/
float StyleSheet::toLength(const TString &value, bool &pixels) {
    pixels = (value.back() != '%');

    TString sr = value.mid(0, value.size() - (pixels ? 2 : 1));
    return sr.toFloat();
}
