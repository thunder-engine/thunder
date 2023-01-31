#ifndef ACTIONS_H
#define ACTIONS_H

#include <editor/propertyedit.h>

#include "metaproperty.h"

class QMenu;
class Object;

namespace Ui {
    class Actions;
}

class Actions : public PropertyEdit {
    Q_OBJECT

public:
    explicit Actions(QWidget *parent = nullptr);
    ~Actions();

    void setMenu(QMenu *menu);
    void setObject(Object *object, const QString &name);

    bool isChecked() const;

public slots:
    void onDataChanged(bool);

private:
    Ui::Actions *ui;

    MetaProperty m_property;

    Object *m_object;

    bool m_menu;

};

#endif // ACTIONS_H
