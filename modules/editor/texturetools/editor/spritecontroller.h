#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/viewport/cameracontroller.h>

#include "../converter/textureconverter.h"

#include "tools/spritetool.h"

#include "spriteedit.h"

class SpriteController : public CameraController {
    Q_OBJECT

public:
    SpriteController(SpriteEdit *editor);

    TextureImportSettings *settings() const { return m_settings; }
    void setSettings(TextureImportSettings *settings);

    void setSize(uint32_t width, uint32_t height);
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    bool isSelected(const TString &key) const;
    void selectElement(const TString &key);
    TString selectedElement();

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Vector3 world() const;

    UndoStack *undoRedo() const { return m_editor->undoRedo(); }

signals:
    void objectsSelected(Object::ObjectList);

    void updated();

private:
    void drawHandles() override;

private:
    TString m_key;

    TextureImportSettings *m_settings;

    EditorTool *m_spriteTool;

    SpriteEdit *m_editor;

    uint32_t m_width;
    uint32_t m_height;

    Vector3 m_world;

    bool m_drag;
};


#endif // SPRITECONTROLLER_H
