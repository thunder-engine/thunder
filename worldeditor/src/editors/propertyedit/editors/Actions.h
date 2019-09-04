#ifndef ACTIONS_H
#define ACTIONS_H

#include <QWidget>

class QMenu;

namespace Ui {
    class Actions;
}

class Actions : public QWidget {
    Q_OBJECT

public:
    explicit Actions (QWidget *parent = nullptr);
    ~Actions ();

    void setMenu (QMenu *menu);

private:
    Ui::Actions *ui;
};

#endif // ACTIONS_H
