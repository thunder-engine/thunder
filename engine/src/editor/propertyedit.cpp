#include "editor/propertyedit.h"

#include <QVariant>

QList<PropertyEdit::UserTypeCallback> PropertyEdit::m_userCallbacks;

PropertyEdit::PropertyEdit(QWidget *parent) :
        QWidget(parent),
        m_propertyObject(nullptr) {

}

PropertyEdit::~PropertyEdit() {

}

QVariant PropertyEdit::data() const {
    return QVariant();
}

void PropertyEdit::setData(const QVariant &data) {
    Q_UNUSED(data)
}

void PropertyEdit::setEditorHint(const QString &hint) {
    Q_UNUSED(hint)
}

void PropertyEdit::setObject(QObject *object, const QString &name) {
    m_propertyName = name;
    m_propertyObject = object;
}

void PropertyEdit::registerEditorFactory(UserTypeCallback callback) {
    if(!m_userCallbacks.contains(callback)) {
        m_userCallbacks.push_back(callback);
    }
}

void PropertyEdit::unregisterEditorFactory(UserTypeCallback callback) {
    int index = m_userCallbacks.indexOf(callback);
    if(index != -1) {
        m_userCallbacks.removeAt(index);
    }
}

PropertyEdit *PropertyEdit::constructEditor(int userType, QWidget *parent, const QString &name, QObject *object) {
    PropertyEdit *result = nullptr;
    if(!m_userCallbacks.isEmpty()) {
        auto iter = m_userCallbacks.begin();
        while(result == nullptr && iter != m_userCallbacks.end() ) {
            result = (*iter)(userType, parent, name, object);
            ++iter;
        }
    }
    return result;
}
