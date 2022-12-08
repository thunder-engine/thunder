#ifndef COMPONENTSELECT_H
#define COMPONENTSELECT_H

#include <QWidget>

#include <components/actor.h>
#include <components/component.h>

namespace Ui {
    class ComponentSelect;
}

struct SceneComponent {
    SceneComponent() :
            component(nullptr),
            actor(nullptr),
            scene(nullptr) {

    }

    QString type;
    Component *component;
    Actor *actor;
    Object *scene;
};
Q_DECLARE_METATYPE(SceneComponent)

class ComponentSelect : public QWidget {
    Q_OBJECT

public:
    explicit ComponentSelect(QWidget *parent = nullptr);
    ~ComponentSelect();

    SceneComponent data() const;
    void setData(const SceneComponent &component);

signals:
    void componentChanged(SceneComponent component);

private slots:
    void onSceneDialog();

    void onClear();

    void onFocused(Object *object);

private:
    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    Ui::ComponentSelect *ui;

    SceneComponent m_component;

    QAction *m_clearAction;
};

#endif // COMPONENTSELECT_H
