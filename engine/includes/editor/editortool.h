#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <engine.h>

#include <cstdint>
#include <QCursor>

class ENGINE_EXPORT EditorTool {
public:
    EditorTool();

    virtual void update(bool center, bool local, bool snap);

    virtual void beginControl();
    virtual void endControl();
    virtual void cancelControl();

    virtual QString icon() const = 0;
    virtual QString name() const = 0;
    virtual QString toolTip() const;
    virtual QString shortcut() const;

    float snap() const;
    void setSnap(float snap);

    Qt::CursorShape cursor() const;

protected:
    Qt::CursorShape m_cursor;

    float m_snap;

};

#endif // EDITORTOOL_H
