#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectController;
class Renderable;

class SelectTool : public EditorTool {
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
        Vector3 pivot;
        Quaternion quat;
        AABBox box;
    };

    typedef QList<Select> SelectList;

public:
    explicit SelectTool(ObjectController *controller);

    void beginControl() override;
    void cancelControl() override;

    QString icon() const override;
    QString name() const override;

    Vector3 objectPosition();
    AABBox objectBound();

    const VariantList &cache() const;

protected:
    VariantList m_propertiesCache;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectController *m_controller;

};

#endif // SELECTTOOL_H
