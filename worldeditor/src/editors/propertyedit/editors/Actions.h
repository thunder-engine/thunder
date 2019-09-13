#ifndef ACTIONS_H
#define ACTIONS_H

#include <QWidget>

class QMenu;
class Object;
class Component;
class Actor;

namespace Ui {
    class Actions;
}

class Actions : public QWidget {
    Q_OBJECT

public:
    explicit Actions (QWidget *parent = nullptr);
    ~Actions ();

    void setMenu (QMenu *menu);
    void setObject (Object *object);

private slots:
    void onDataChanged (int);

private:
    Ui::Actions *ui;
    Component *m_pComponent;
    Actor *m_pActor;
};

#endif // ACTIONS_H
