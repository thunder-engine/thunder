#ifndef EDITORGADGET_H
#define EDITORGADGET_H

#include <QWidget>

#include <editor.h>

class AssetEditor;

class EDITOR_EXPORT EditorGadget : public QWidget {
    Q_OBJECT

public:
    explicit EditorGadget(QWidget *parent = nullptr);

    virtual void setCurrentEditor(AssetEditor *editor) {};

signals:
    void updated();

    void objectsSelected(const Object::ObjectList &objects, bool force);
    void objectsChanged(const Object::ObjectList &objects, const TString &property, Variant value);

public slots:
    virtual void onUpdated() = 0;

    virtual void onObjectsSelected(const Object::ObjectList &objects) = 0;
    virtual void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) = 0;

};

#endif // EDITORGADGET_H
