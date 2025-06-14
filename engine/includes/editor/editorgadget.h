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

    void objectsSelected(std::list<Object *> objects, bool force);
    void objectsChanged(std::list<Object *> objects, const QString property);

public slots:
    virtual void onUpdated() = 0;

    virtual void onItemsSelected(std::list<QObject *> items) = 0;

    virtual void onObjectsSelected(std::list<Object *> objects) = 0;
    virtual void onObjectsChanged(std::list<Object *> objects, const QString property, Variant value) = 0;

};

#endif // EDITORGADGET_H
