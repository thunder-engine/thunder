#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <editor/propertyedit.h>

class ColorEdit : public PropertyEdit {
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = nullptr);

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    Vector4 m_color;

    QBrush m_brush;

};

#endif // COLOREDIT_H
