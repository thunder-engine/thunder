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

    typedef QMap<uint32_t, Select> SelectMap;

public:
    explicit EditorTool(EditorTool::SelectMap &selection);

    virtual void update();

    virtual void beginControl();
    virtual void endControl();
    virtual void cancelControl();

    virtual QString icon() const = 0;
    virtual QString name() const = 0;

    QCursor cursor() const;

    Vector3 objectPosition();
    AABBox objectBound();

    const VariantList &cache() const;

protected:
    SelectMap &m_Selected;

    VariantList m_PropertiesCache;

    QCursor m_Cursor;

};

#endif // EDITORTOOL_H
