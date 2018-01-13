#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <QToolButton>

class ColorEdit : public QToolButton {
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = 0);

    QColor          color               () const;
    void            setColor            (const QString &c);

signals:
    void            colorChanged        (const QString);

private slots:
    void            colorPickDlg        ();

private:
    void            paintEvent          (QPaintEvent *);

    QColor          mColor;

    QBrush          mBrush;
};

#endif // COLOREDIT_H
