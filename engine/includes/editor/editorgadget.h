#ifndef EDITORGADGET_H
#define EDITORGADGET_H

#include <engine.h>

#include <QWidget>

class ENGINE_EXPORT EditorGadget : public QWidget {
    Q_OBJECT

public:
    explicit EditorGadget(QWidget *parent = nullptr);

signals:
    void updated();

    void objectsSelected(Object::ObjectList objects);

public slots:
    virtual void onObjectsSelected(Object::ObjectList objects) = 0;

    virtual void onObjectsChanged(Object::ObjectList objects, const QString property) = 0;

};

#endif // EDITORGADGET_H
