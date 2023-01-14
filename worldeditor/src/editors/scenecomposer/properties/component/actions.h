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
    explicit Actions(const QString &name, QWidget *parent = nullptr);
    ~Actions();

    void setMenu(QMenu *menu);
    void setObject(Object *object);

    bool isChecked() const;

public slots:
    void onDataChanged(bool);

private:
    Ui::Actions *ui;

    MetaProperty m_property;

    QString m_name;

    Object *m_object;

    bool m_menu;

};

#endif // ACTIONS_H
