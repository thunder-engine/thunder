#ifndef LABEL_H
#define LABEL_H

#include "components/widget.h"

class Font;
class Material;

class LabelPrivate;

class Label : public Widget {
    A_REGISTER(Label, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(string, text, Label::text, Label::setText),
        A_PROPERTYEX(int, alignment, Label::align, Label::setAlign, "editor=Alignment"),
        A_PROPERTYEX(Font *, font, Label::font, Label::setFont, "editor=Template"),
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
    void setText(const string &text);

    Font *font() const;
    void setFont(Font *font);

    int fontSize() const;
    void setFontSize(int size);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    bool wordWrap() const;
    void setWordWrap(bool wrap);

    int align() const;
    void setAlign(int alignment);

    bool kerning() const;
    void setKerning(const bool kerning);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadData(const VariantList &data) override;
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    bool event(Event *ev) override;

    void boundChanged() override;

private:
    LabelPrivate *p_ptr;

};

#endif // LABEL_H
