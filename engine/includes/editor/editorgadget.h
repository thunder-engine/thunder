#ifndef EDITORGADGET_H
#define EDITORGADGET_H

#include <engine.h>

#include <QWidget>

class AssetEditor;

class ENGINE_EXPORT EditorGadget : public QWidget {
    Q_OBJECT

public:
    explicit EditorGadget(QWidget *parent = nullptr);

    virtual void setCurrentEditor(AssetEditor *editor) {};

signals:
    void updated();

    void objectsSelected(QList<Object *> objects, bool force);
    void objectsChanged(QList<Object *> objects, const QString property);

public slots:
    virtual void onUpdated() = 0;

    virtual void onItemsSelected(QList<QObject *> items) = 0;

    virtual void onObjectsSelected(QList<Object *> objects) = 0;
    virtual void onObjectsChanged(QList<Object *> objects, const QString property, Variant value) = 0;

};

#endif // EDITORGADGET_H
