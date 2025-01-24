#ifndef WIDGETTOOL_H
#define WIDGETTOOL_H

#include <editor/editortool.h>

class WidgetController;
class Renderable;

class WidgetTool : public EditorTool {
public:
    struct Select {
        bool operator==(const Select &left) {
            return (uuid == left.uuid);
        }

        uint32_t uuid = 0;
        Actor *object = nullptr;
        Renderable *renderable = nullptr;

        Vector3 position;
        Vector3 scale;
        Vector3 euler;
        Quaternion quat;
    };

    typedef QList<Select> SelectList;

public:
    explicit WidgetTool(WidgetController *controller);

    Vector3 objectPosition();
    AABBox objectBound();

protected:
    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

protected:
    WidgetController *m_controller;

    AABBox m_savedBox;

    AABBox m_box;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

};

#endif // WIDGETTOOL_H
