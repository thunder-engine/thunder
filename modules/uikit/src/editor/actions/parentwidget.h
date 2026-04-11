#ifndef PARENTWIDGET_H
#define PARENTWIDGET_H

#include "../widgetcontroller.h"

class ParentWidget : public UndoCommand {
public:
    ParentWidget(const Object::ObjectList &objects, Object *parent, int32_t position, WidgetController *ctrl, const TString &name = "Parent Change", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::unordered_map<uint32_t, uint32_t> m_parentCache;
    std::list<uint32_t> m_objects;

    uint32_t m_parent;

    int32_t m_position;

    WidgetController *m_controller;

};

#endif // PARENTWIDGET_H
