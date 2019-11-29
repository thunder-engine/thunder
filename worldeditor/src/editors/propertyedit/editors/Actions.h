#ifndef ACTIONS_H
#define ACTIONS_H

#include <QWidget>

#include "metaproperty.h"

class QMenu;
class Object;

namespace Ui {
    class Actions;
}

class Actions : public QWidget {
    Q_OBJECT

public:
    explicit Actions (const QString &name, QWidget *parent = nullptr);
    ~Actions ();

    void setMenu (QMenu *menu);
    void setObject (Object *object);

private slots:
    void onDataChanged (int);

private:
    Ui::Actions *ui;

    bool m_Menu;
    QString m_Name;
    Object *m_pObject;
    MetaProperty m_Property;
};

#endif // ACTIONS_H
