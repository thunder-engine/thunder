#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <engine.h>

#include <cstdint>
#include <QMap>
#include <QCursor>

class Actor;

class NEXT_LIBRARY_EXPORT EditorTool {
public:
    struct Select {
        Actor *object;
        Vector3 position;
        Vector3 scale;
        Vector3 euler;
    };

    typedef QMap<uint32_t, Select> SelectMap;

public:
    explicit EditorTool(EditorTool::SelectMap &selection);

    virtual void update();

    virtual void beginControl();
    virtual void endControl();

    virtual QString icon() const = 0;
    virtual QString name() const = 0;

    QCursor cursor() const;

    Vector3 objectPosition();
    AABBox objectBound();

protected:
    SelectMap &m_Selected;

    QCursor m_Cursor;

};

#endif // EDITORTOOL_H
