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

    void objectsSelected(QList<Object *> objects);

public slots:
    virtual void onItemsSelected(QList<QObject *> objects) = 0;
    virtual void onObjectsSelected(QList<Object *> objects) = 0;

    virtual void onObjectsChanged(QList<Object *> objects, const QString property) = 0;

};

#endif // EDITORGADGET_H
