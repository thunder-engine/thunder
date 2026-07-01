#ifndef EDITORTOOL_H
#define EDITORTOOL_H

#include <editor.h>

#include <QWidget>

class EDITOR_EXPORT EditorTool {
public:
    EditorTool();
    virtual ~EditorTool();

    virtual void update(bool center, bool local, bool snap);

    virtual void beginControl();
    virtual void endControl();
    virtual void cancelControl();

    virtual TString icon() const = 0;
    virtual TString name() const = 0;
    virtual TString toolTip() const;
    virtual TString shortcut() const;

    virtual TString component() const;

    virtual bool blockSelection() const;

    virtual QWidget *panel();

protected:
    VariantList m_propertiesCache;

};

#endif // EDITORTOOL_H
