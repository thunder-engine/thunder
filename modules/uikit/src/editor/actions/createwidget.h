#ifndef CREATEWIDGET_H
#define CREATEWIDGET_H

#include "../widgetcontroller.h"

class CreateWidget : public UndoCommand {
public:
    CreateWidget(const TString &type, Scene *scene, WidgetController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;

    TString m_type;

    WidgetController *m_controller;

};

#endif // CREATEWIDGET_H
