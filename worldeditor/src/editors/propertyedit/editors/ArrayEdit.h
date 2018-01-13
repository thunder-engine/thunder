#ifndef ARRAYEDIT_H
#define ARRAYEDIT_H

#include <QLineEdit>
#include <QToolButton>

#include <amath.h>

class ArrayEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit        ArrayEdit           (QWidget *parent = 0);

signals:
    void            elementAdded        ();
    void            allCleared          ();
    
private slots:
    void            addElement          ();
    void            clearAll            ();

private:
    void            resizeEvent         (QResizeEvent *event);

    QToolButton    *mAddBtn;
    QToolButton    *mClearBtn;
};

#endif // ARRAYEDIT_H
