#ifndef ARRAYEDIT_H
#define ARRAYEDIT_H

#include <QLineEdit>
#include <QToolButton>

#include <amath.h>

class ArrayEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit ArrayEdit (QWidget *parent = nullptr);

signals:
    void elementAdded ();
    void allCleared ();
    
private:
    void resizeEvent (QResizeEvent *event);

    QToolButton *m_pAddBtn;
    QToolButton *m_pClearBtn;
};

#endif // ARRAYEDIT_H
