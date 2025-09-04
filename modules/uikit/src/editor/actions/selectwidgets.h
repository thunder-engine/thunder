#ifndef SEKECTWIDGETS_H
#define SEKECTWIDGETS_H

#include "../widgetcontroller.h"

class SelectWidgets : public UndoCommand {
public:
    SelectWidgets(const std::list<uint32_t> &objects, WidgetController *ctrl, const TString &name = "Selection Change", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

    WidgetController *m_controller;

};

#endif // SEKECTOBJECTS_H
