#ifndef PASTEWIDGET_H
#define PASTEWIDGET_H

#include "../widgetcontroller.h"

class PasteWidget : public UndoCommand {
public:
    PasteWidget(WidgetController *ctrl, const TString &name = "Paste Widget", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    Variant m_data;

    std::unordered_map<uint32_t, uint32_t> m_uuidBinds;

    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;

    WidgetController *m_controller;

};

#endif // PASTEWIDGET_H
