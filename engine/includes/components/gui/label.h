#ifndef LABEL_H
#define LABEL_H

#include <widget.h>

#include <font.h>

class Mesh;
class Material;
class MaterialInstance;

class ENGINE_EXPORT Label : public Widget {
    A_REGISTER(Label, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(string, text, Label::text, Label::setText),
        A_PROPERTYEX(int, alignment, Label::align, Label::setAlign, "editor=Alignment"),
        A_PROPERTYEX(Font *, font, Label::font, Label::setFont, "editor=Asset"),
        A_PROPERTY(int, fontSize, Label::fontSize, Label::setFontSize),
        A_PROPERTYEX(Vector4, color, Label::color, Label::setColor, "editor=Color"),
        A_PROPERTY(bool, wordWrap, Label::wordWrap, Label::setWordWrap),
        A_PROPERTY(bool, kerning, Label::kerning, Label::setKerning)
    )
    A_NOMETHODS()

public:
    Label();
    ~Label();

    string text() const;
    void setText(const string text);

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

    void loadData(const VariantList &data) override;
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    bool event(Event *ev) override;

    void boundChanged(const Vector2 &size) override;

    void composeComponent() override;

    static void fontUpdated(int state, void *ptr);

private:
    string m_text;

    Vector4 m_color;

    Vector2 m_meshSize;

    Vector2 m_clipOffset;

    Font *m_font;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    int32_t m_size;

    int m_alignment;

    bool m_kerning;

    bool m_wrap;

};

#endif // LABEL_H
