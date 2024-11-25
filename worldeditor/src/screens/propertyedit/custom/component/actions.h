#ifndef ACTIONS_H
#define ACTIONS_H

#include <editor/propertyedit.h>

class Object;

class PropertyEditor;

namespace Ui {
    class Actions;
}

class Actions : public PropertyEdit {
    Q_OBJECT

public:
    explicit Actions(QWidget *parent = nullptr);
    ~Actions();

    void setObject(Object *object, const QString &name);

    void setObject(QObject *object, const QString &name) override;

    bool isChecked() const;

public slots:
    void onDataChanged(bool);

private:
    PropertyEditor *findEditor(QWidget *parent) const;

    Ui::Actions *ui;

    MetaProperty m_property;

    Object *m_object;

};

#endif // ACTIONS_H
