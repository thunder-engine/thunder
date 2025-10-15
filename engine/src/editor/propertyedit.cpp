#include "editor/propertyedit.h"

std::list<PropertyEdit::UserTypeCallback> PropertyEdit::m_userCallbacks;

PropertyEdit::PropertyEdit(QWidget *parent) :
        QWidget(parent),
        m_object(nullptr) {

}

PropertyEdit::~PropertyEdit() {

}

Variant PropertyEdit::data() const {
    return Variant();
}

void PropertyEdit::setData(const Variant &data) {
    A_UNUSED(data);
}

void PropertyEdit::setEditorHint(const TString &hint) {
    A_UNUSED(hint);
}

void PropertyEdit::setObject(Object *object, const TString &) {
    m_object = object;
}

void PropertyEdit::registerEditorFactory(UserTypeCallback callback) {
    m_userCallbacks.push_back(callback);
}

void PropertyEdit::unregisterEditorFactory(UserTypeCallback callback) {
    m_userCallbacks.remove(callback);
}

PropertyEdit *PropertyEdit::constructEditor(int userType, QWidget *parent, const TString &editor) {
    PropertyEdit *result = nullptr;
    if(!m_userCallbacks.empty()) {
        auto iter = m_userCallbacks.begin();
        while(result == nullptr && iter != m_userCallbacks.end() ) {
            result = (*iter)(userType, parent, editor);
            ++iter;
        }
    }
    return result;
}
