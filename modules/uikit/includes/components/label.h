#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

#include <font.h>

class Mesh;
class Material;
class MaterialInstance;

class UIKIT_EXPORT Label : public Widget {
    A_REGISTER(Label, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(string, text, Label::text, Label::setText),
        A_PROPERTYEX(int, alignment, Label::align, Label::setAlign, "editor=Alignment, css=text-align"),
        A_PROPERTYEX(Font *, font, Label::font, Label::setFont, "editor=Asset"),
        A_PROPERTYEX(int, fontSize, Label::fontSize, Label::setFontSize, "css=font-size"),
        A_PROPERTYEX(Vector4, color, Label::color, Label::setColor, "editor=Color, css=color"),
        A_PROPERTYEX(bool, wordWrap, Label::wordWrap, Label::setWordWrap, "css=white-space"),
        A_PROPERTYEX(bool, kerning, Label::kerning, Label::setKerning, "css=font-kerning")
    )
    A_NOMETHODS()

public:
    Label();
    ~Label();

    std::string text() const;
    void setText(const std::string text);

    Font *font() const;
    void setFont(Font *font);

    int fontSize() const;
    void setFontSize(int size);

    Vector4 color() const;
    void setColor(const Vector4 color);

    bool wordWrap() const;
    void setWordWrap(bool wrap);

    int align() const;
    void setAlign(int alignment);

    bool kerning() const;
    void setKerning(const bool kerning);

    Vector2 cursorAt(int position);

    void setClipOffset(const Vector2 &offset);

private:
    void draw(CommandBuffer &buffer) override;
    void applyStyle() override;

    void loadData(const VariantList &data) override;
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    bool event(Event *ev) override;

    void boundChanged(const Vector2 &size) override;

    void composeComponent() override;

    static void fontUpdated(int state, void *ptr);

private:
    std::string m_text;

    Vector4 m_color;

    Vector2 m_meshSize;

    Vector2 m_clipOffset;

    Font *m_font;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    int32_t m_size;

    int m_alignment;

    float m_fontWeight;

    bool m_kerning;

    bool m_wrap;

    bool m_dirty;

};

#endif // LABEL_H
