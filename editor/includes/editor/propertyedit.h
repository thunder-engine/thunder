/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef PROPERTYEDIT_H
#define PROPERTYEDIT_H

#include <editor.h>
#include <widget.h>

#include <list>

class Label;
class Layout;

class EDITOR_EXPORT PropertyEdit : public Widget {
    A_OBJECT(PropertyEdit, Widget, Editor)
    A_METHODS(
        A_SIGNAL(PropertyEdit::dataChanged),
        A_SIGNAL(PropertyEdit::editFinished),
        A_SIGNAL(PropertyEdit::selectionRequested)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    typedef PropertyEdit *(*UserTypeCallback)(uint32_t userType, Widget *parent, const TString &editor);

    PropertyEdit();
    ~PropertyEdit() override;

    virtual Variant data() const;
    virtual void setData(const Variant &data);
    virtual void setEditorHint(const TString &hint);
    virtual void setObject(Object *object, const TString &property);
    virtual void setObjects(const Object::ObjectList &objects, const TString &property);
    virtual void setMixedValue(bool mixed);
    virtual bool setSelectedObject(Object *object);

    TString propertyName() const;
    Object *editedObject() const;
    const Object::ObjectList &editedObjects() const;

    void dataChanged();
    void editFinished();
    void selectionRequested();

    static void registerEditorFactory(UserTypeCallback callback);
    static void unregisterEditorFactory(UserTypeCallback callback);
    static PropertyEdit *constructEditor(uint32_t userType, Widget *parent, const TString &editor);

    static bool parseFloat(const TString &text, float &value);
    static bool parseInteger(const TString &text, int &value);

    static TString propertyDisplayName(const TString &name);

protected:
    void addEditor(Widget *editor);
    Layout *editorLayout() const;

    Object *m_object;
    TString m_property;
    Object::ObjectList m_objects;
    bool m_mixedValue;

    Label *m_label;
    Layout *m_editorLayout;

private:
    static std::list<UserTypeCallback> m_userCallbacks;
};

#endif // PROPERTYEDIT_H
