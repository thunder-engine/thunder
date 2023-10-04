#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <engine.h>

#include <cstdint>
#include <QMap>
#include <QCursor>

class Actor;
class Renderable;

class ENGINE_EXPORT EditorTool {
public:
    struct ENGINE_EXPORT Select {
        Select();

        bool operator==(const Select &left) {
            return (uuid == left.uuid);
        }

        uint32_t uuid;
        Actor *object;
        Renderable *renderable;
        Vector3 position;
        Vector3 scale;
        Vector3 euler;
        Vector3 pivot;
        AABBox box;
    };

    typedef QList<Select> SelectList;

public:
    explicit EditorTool(EditorTool::SelectList &selection);

    virtual void update(bool pivot, bool local, float snap);

    virtual void beginControl();
    virtual void endControl();
    virtual void cancelControl();

    virtual QString icon() const = 0;
    virtual QString name() const = 0;
    virtual QString toolTip() const;
    virtual QString shortcut() const;

    Qt::CursorShape cursor() const;

    Vector3 objectPosition();
    AABBox objectBound();

    const VariantList &cache() const;

protected:
    SelectList &m_selected;

    VariantList m_propertiesCache;

    Qt::CursorShape m_cursor;

};

#endif // EDITORTOOL_H
