#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <components/widget.h>

class Image;

class ProgressBarPrivate;

class ProgressBar : public Widget {
    A_REGISTER(ProgressBar, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(float, from, ProgressBar::from, ProgressBar::setFrom),
        A_PROPERTY(float, to, ProgressBar::to, ProgressBar::setTo),
        A_PROPERTY(float, value, ProgressBar::value, ProgressBar::setValue),
        A_PROPERTYEX(Image *, backgroundGraphic, ProgressBar::backgroundGraphic, ProgressBar::setBackgroundGraphic, "editor=Component"),
        A_PROPERTYEX(Image *, progressGraphic, ProgressBar::progressGraphic, ProgressBar::setProgressGraphic, "editor=Component"),
        A_PROPERTYEX(Vector4, backgroundColor, ProgressBar::backgroundColor, ProgressBar::setBackgroundColor, "editor=Color"),
        A_PROPERTYEX(Vector4, progressColor, ProgressBar::progressColor, ProgressBar::setProgressColor, "editor=Color")
    )
    A_NOMETHODS()

public:
    ProgressBar();
    ~ProgressBar();

    float from() const;
    void setFrom(float value);

    float to() const;
    void setTo(float value);

    float value() const;
    void setValue(float value);

    Image *backgroundGraphic() const;
    void setBackgroundGraphic(Image *image);

    Image *progressGraphic() const;
    void setProgressGraphic(Image *image);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 &color);

    Vector4 progressColor() const;
    void setProgressColor(const Vector4 &color);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void composeComponent() override;

    void onReferenceDestroyed() override;

private:
    ProgressBarPrivate *p_ptr;

};

#endif // PROGRESSBAR_H
