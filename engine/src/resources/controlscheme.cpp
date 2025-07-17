#include "resources/controlscheme.h"

#include <input.h>

namespace  {
    const char *gData = "Data";
    const char *gName = "Name";
    const char *gCode = "Code";
    const char *gNegative = "Negative";
    const char *gBindings = "Bindings";
}

/*!
    \class ControlScheme
    \brief The ControlScheme class manages the mapping of actions to input bindings, allowing for customization of control schemes in a game.
    \inmodule Resources

    The ControlScheme class provides functionality for managing and customizing control schemes within a game.
    It allows users to define actions and map them to various input bindings.
*/

/*!
    Gets the total number of actions in the control scheme.
*/
int ControlScheme::actionsCount() const {
    return m_actions.size();
}
/*!
    Returns The name of the specified \a action or an empty string if the index is out of range.
*/
TString ControlScheme::actionName(int action) const {
    if(action >= m_actions.size()) {
        return TString();
    }
    return m_actions[action].name;
}
/*!
    Returns The number of bindings for the specified \a action.
*/
int ControlScheme::bindingsCount(int action) const {
    if(action >= m_actions.size()) {
        return 0;
    }
    return m_actions[action].bindings.size();
}
/*!
    Returns the input code for the specified \a binding \a action or Input::KEY_UNKNOWN if the indices are out of range.
*/
int ControlScheme::bindingCode(int action, int binding) {
    if(action >= m_actions.size() || binding >= m_actions[action].bindings.size()) {
        return Input::KEY_UNKNOWN;
    }
    return m_actions[action].bindings[binding].code;
}
/*!
    Returns true if the \a binding \a action is negative, false otherwise. Returns false if the indices are out of range.
*/
bool ControlScheme::bindingNegative(int action, int binding) {
    if(action >= m_actions.size() || binding >= m_actions[action].bindings.size()) {
        return false;
    }
    return m_actions[action].bindings[binding].negative;
}
/*!
    \internal
*/
void ControlScheme::loadUserData(const VariantMap &data) {
    m_actions.clear();

    auto it = data.find(gData);
    if(it != data.end()) {
        VariantList list = it->second.value<VariantList>();
        m_actions.reserve(list.size());
        for(auto &act : list) {
            Action action;
            VariantMap actions = act.value<VariantMap>();
            action.name = actions[gName].toString();

            VariantList bindings = actions[gBindings].value<VariantList>();
            action.bindings.reserve(bindings.size());
            for(auto &bnd : bindings) {
                VariantMap fields = bnd.value<VariantMap>();

                Binding binding;
                binding.path = fields[gCode].toString();
                binding.code = Input::getCode(binding.path);
                binding.negative = fields[gNegative].toBool();

                action.bindings.push_back(binding);
            }
            m_actions.push_back(action);
        }
    }
}
/*!
    \internal
*/
VariantMap ControlScheme::saveUserData() const {
    VariantMap result;

    VariantList list;
    for(auto &act : m_actions) {
        VariantMap action;
        action[gName] = act.name;

        VariantList bindings;
        for(auto &bnd : act.bindings) {
            VariantMap binding;
            binding[gCode] = bnd.path;
            binding[gNegative] = bnd.negative;

            bindings.push_back(binding);
        }
        action[gBindings] = bindings;

        list.push_back(action);
    }

    result[gData] = list;

    return result;
}
