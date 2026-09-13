#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <editor/propertyedit.h>

class ColorEdit : public PropertyEdit {
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = nullptr);

    Variant data() const override;
    void setData(const Variant &data) override;
    void setMixedValue(bool mixed) override;

private:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    Vector4 m_color;
    bool m_mixed = false;

    QBrush m_brush;

};

#endif // COLOREDIT_H
