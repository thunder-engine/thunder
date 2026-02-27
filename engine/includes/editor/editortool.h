#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <engine.h>

#include <QWidget>

class ENGINE_EXPORT EditorTool {
public:
    EditorTool();
    virtual ~EditorTool();

    virtual void update(bool center, bool local, bool snap);

    virtual void beginControl();
    virtual void endControl();
    virtual void cancelControl();

    virtual std::string icon() const = 0;
    virtual std::string name() const = 0;
    virtual std::string toolTip() const;
    virtual std::string shortcut() const;

    virtual std::string component() const;

    virtual bool blockSelection() const;

    virtual QWidget *panel();

protected:
    VariantList m_propertiesCache;

};

#endif // EDITORTOOL_H
