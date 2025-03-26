#include "editor/editortool.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/renderable.h"
#include "components/camera.h"

#include "editor/viewport/handletools.h"

/*!
    \class EditorTool
    \brief The EditorTool class is an abstract base class that defines the interface for editor tools in the application.
    \inmodule Editor

    The EditorTool class is an abstract base class that defines the interface for editor tools in the application.
    It provides common functionality and virtual methods that must be implemented by specific tool implementations.

    Example:
    \code
        class MoveTool : public EditorTool {
        public:
            std::string icon() const override {
                return ":/Images/editor/Move.png";
            }

            std::string name() const override {
                return "Move Tool";
            }

            std::string toolTip() const override {
                return "Moves selected objects";
            }

            std::string shortcut() const override {
                return "Shift+T";
            }

            void update(bool center, bool local, bool snap) override {
                // Implement move tool logic
            }

            // ... other overrides as needed
        };
*/

EditorTool::EditorTool() :
        m_cursor(Qt::ArrowCursor) {

}

EditorTool::~EditorTool() {

}
/*!
    Returns the tooltip text for the tool (default implementation returns empty string).
*/
std::string EditorTool::toolTip() const {
    return std::string();
}
/*!
    Returns the keyboard shortcut for the tool (default implementation returns empty string).
*/
std::string EditorTool::shortcut() const {
    return std::string();
}
/*!
    Returns the associated component type for the tool (default implementation returns empty string).
*/
std::string EditorTool::component() const {
    return std::string();
}
/*!
    Determines whether the tool blocks regular selection behavior (default returns false).
*/
bool EditorTool::blockSelection() const {
    return false;
}
/*!
    Returns a custom panel widget for tool options (default implementation returns nullptr).
*/
QWidget *EditorTool::panel() {
    return nullptr;
}
/*!
    Updates the controled Component state and performs any necessary operations.
    Parameter \a center used for center-based or origin-based transformations.
    Parameter \a local used for local space or world space coordinates.
    Parameter \a snap enables stepping transform operations.
*/
void EditorTool::update(bool center, bool local, bool snap) {
    A_UNUSED(center);
    A_UNUSED(local);
    A_UNUSED(snap);
}

/*!
    Called when the tool begins active control.
    Can be used to store the initial state of controlled Component.
*/
void EditorTool::beginControl() {

}
/*!
    Called when the tool ends active control.
    Can be used to store current operation to Undo/Redo system.
*/
void EditorTool::endControl() {

}
/*!
    Cancels the current control operation.
    Can be used to restore the state of contolled Component.

    \sa EditorTool::beginControl()
*/
void EditorTool::cancelControl() {

}
/*!
    Returns the icon identifier for the tool.
*/
Qt::CursorShape EditorTool::cursor() const {
    return m_cursor;
}
