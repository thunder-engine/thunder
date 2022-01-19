#include "resources/controlscheme.h"

#include <input.h>

#define DATA        "Data"
#define NAME        "Name"
#define CODE        "Code"
#define NEGATIVE    "Negative"
#define BINDINGS    "Bindings"

int ControlScheme::actionsCount() const {
    return m_actions.size();
}

string ControlScheme::actionName(int action) const {
    if(action >= m_actions.size()) {
        return string();
    }
    return m_actions[action].name;
}

int ControlScheme::bindingsCount(int action) const {
    if(action >= m_actions.size()) {
        return 0;
    }
    return m_actions[action].bindings.size();
}

int ControlScheme::bindingCode(int action, int binding) {
    if(action >= m_actions.size() || binding >= m_actions[action].bindings.size()) {
        return Input::KEY_UNKNOWN;
    }
    return m_actions[action].bindings[binding].code;
}

bool ControlScheme::bindingNegative(int action, int binding) {
    if(action >= m_actions.size() || binding >= m_actions[action].bindings.size()) {
        return false;
    }
    return m_actions[action].bindings[binding].negative;
}

void ControlScheme::loadUserData(const VariantMap &data) {
    m_actions.clear();

    auto it = data.find(DATA);
    if(it != data.end()) {
        VariantList list = it->second.value<VariantList>();
        m_actions.reserve(list.size());
        for(auto &act : list) {
            Action action;
            VariantMap actions = act.value<VariantMap>();
            action.name = actions[NAME].toString();

            VariantList bindings = actions[BINDINGS].value<VariantList>();
            action.bindings.reserve(bindings.size());
            for(auto &bnd : bindings) {
                VariantMap fields = bnd.value<VariantMap>();

                Binding binding;
                binding.path = fields[CODE].toString();
                binding.code = Input::getCode(binding.path);
                binding.negative = fields[NEGATIVE].toBool();

                action.bindings.push_back(binding);
            }
            m_actions.push_back(action);
        }
    }
}

VariantMap ControlScheme::saveUserData() const {
    VariantMap result;

    VariantList list;
    for(auto &act : m_actions) {
        VariantMap action;
        action[NAME] = act.name;

        VariantList bindings;
        for(auto &bnd : act.bindings) {
            VariantMap binding;
            binding[CODE] = bnd.path;
            binding[NEGATIVE] = bnd.negative;

            bindings.push_back(binding);
        }
        action[BINDINGS] = bindings;

        list.push_back(action);
    }

    result[DATA] = list;

    return result;
}
