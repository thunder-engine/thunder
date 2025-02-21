#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectController;

class QLineEdit;

class SelectTool : public EditorTool {
public:
    struct Select {
        bool operator==(const Select &left) const {
            return (uuid == left.uuid);
        }

        uint32_t uuid = 0;
        Actor *object = nullptr;
    };

    typedef QList<Select> SelectList;

public:
    explicit
    SelectTool(ObjectController *controller);

    virtual QLineEdit *snapWidget();

    float snap() const;
    void setSnap(float snap);

protected:
    void update(bool center, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

    Vector3 objectPosition();
    AABBox objectBound();

protected:
    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectController *m_controller;

    QLineEdit *m_snapEditor;

    float m_snap;

};

#endif // SELECTTOOL_H
