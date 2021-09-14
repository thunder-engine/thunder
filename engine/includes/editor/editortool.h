#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <engine.h>

#include <cstdint>
#include <QMap>
#include <QCursor>

class Actor;
class Renderable;

class NEXT_LIBRARY_EXPORT EditorTool {
public:
    struct NEXT_LIBRARY_EXPORT Select {
        Select();

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

    QCursor cursor() const;

    Vector3 objectPosition();
    AABBox objectBound();

    const VariantList &cache() const;

protected:
    SelectList &m_Selected;

    VariantList m_PropertiesCache;

    QCursor m_Cursor;

};

#endif // EDITORTOOL_H
