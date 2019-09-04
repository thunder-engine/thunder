#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <QToolButton>

class ColorEdit : public QToolButton {
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = nullptr);

    QColor color () const;
    void setColor (const QString &c);

signals:
    void colorChanged (const QString);

private slots:
    void colorPickDlg ();

private:
    void paintEvent (QPaintEvent *);

    QColor m_Color;

    QBrush m_Brush;
};

#endif // COLOREDIT_H
