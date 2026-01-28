#include "animationcontrollergraph.h"

#include "basestate.h"
#include "entrystate.h"
#include "statelink.h"
#include "animationbuilder.h"

#include <resources/animationstatemachine.h>
#include <pugixml.hpp>

namespace {
    const char *gMachine("Machine");

    const char *gVariables("variables");
    const char *gVariable("variable");
};

StringList AnimationControllerGraph::m_nodeTypes;

AnimationControllerGraph::AnimationControllerGraph() :
        m_entryState(nullptr) {
    m_version = AnimationControllerBuilder::version();

    if(m_nodeTypes.empty()) {
        GraphNode::registerClassFactory(Engine::resourceSystem());
        GraphLink::registerClassFactory(Engine::resourceSystem());
        StateNode::registerClassFactory(Engine::resourceSystem());
        BaseState::registerClassFactory(Engine::resourceSystem());
        EntryState::registerClassFactory(Engine::resourceSystem());
        StateLink::registerClassFactory(Engine::resourceSystem());

        for(auto &it : Engine::factories()) {
            Url url(it.second);

            if(url.host() == "Motion") {
                TString path = url.path();
                if(path.front() == '/') {
                    path.removeFirst();
                }
                m_nodeTypes.push_back(path);
            }
        }
    }
}

GraphNode *AnimationControllerGraph::fallbackRoot() {
    GraphNode *node = Engine::objectCreate<EntryState>("EntryState");
    node->setGraph(this);
    m_nodes.push_front(node);

    return node;
}

void AnimationControllerGraph::onNodesLoaded() {
    m_entryState = nullptr;

    for(auto it : m_nodes) {
        EntryState *entry = dynamic_cast<EntryState *>(it);
        if(entry) {
            m_entryState = entry;
            break;
        }
    }
}

GraphNode *AnimationControllerGraph::nodeCreate(const TString &type, int &index) {
    GraphNode *node = dynamic_cast<GraphNode *>(Engine::objectCreate(type));
    if(node) {
        node->setGraph(this);
        node->setTypeName(type);
        node->setName(type);

        if(index == -1) {
            index = m_nodes.size();
            m_nodes.push_back(node);
        } else {
            m_nodes.insert(std::next(m_nodes.begin(), index), node);
        }
    }
    return node;
}

GraphLink *AnimationControllerGraph::linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) {
    if(receiver == m_entryState) {
        return nullptr;
    }
    if(sender == m_entryState) {
        auto it = m_links.begin();
        while(it != m_links.end()) {
            GraphLink *link = *it;
            if(link->sender == sender) {
                it = m_links.erase(it);
                delete link;
            } else {
                ++it;
            }
        }
    }
    return AbstractNodeGraph::linkCreate(sender, oport, receiver, iport);
}

StringList AnimationControllerGraph::variables() const {
    StringList result;
    for(auto &it : m_variables) {
        result.push_back(it.first);
    }
    return result;
}

Variant AnimationControllerGraph::variable(const TString &name) {
    auto it = m_variables.find(name);
    if(it != m_variables.end()) {
        return it->second;
    }
    return Variant();
}

void AnimationControllerGraph::addVariable(const TString &name, Variant &value) {
    m_variables[name] = value;

    setProperty(name.data(), value);
}

void AnimationControllerGraph::removeVariable(const TString &name) {
    m_variables.erase(name);

    setProperty(name.data(), Variant());
}

void AnimationControllerGraph::variableChanged() {
    emitSignal(_SIGNAL(variableChanged()));
}

GraphLink *AnimationControllerGraph::linkCreate() {
    return Engine::objectCreate<StateLink>();
}

void AnimationControllerGraph::loadGraph(const pugi::xml_node &graph) {
    for(auto &it : m_variables) {
        setProperty(it.first.data(), Variant());
    }
    m_variables.clear();

    pugi::xml_node sub = graph.first_child();
    while(sub) {
        std::string name(sub.name());
        if(name == gVariables) {
            pugi::xml_node variableElement = sub.first_child();
            while(variableElement) {
                TString name(variableElement.attribute(gName).as_string());
                TString type(variableElement.attribute(gType).as_string());
                Variant value;
                if(type == "bool") {
                    value = variableElement.attribute(gValue).as_bool();
                } else if(type == "int") {
                    value = variableElement.attribute(gValue).as_int();
                } else if(type == "float") {
                    value = variableElement.attribute(gValue).as_float();
                }
                addVariable(name, value);
                variableElement = variableElement.next_sibling();
            }
            variableChanged();
        }
        sub = sub.next_sibling();
    }

    AbstractNodeGraph::loadGraph(graph);
}

void AnimationControllerGraph::saveGraph(pugi::xml_node &graph) const {
    AbstractNodeGraph::saveGraph(graph);

    pugi::xml_node variablesElement = graph.append_child(gVariables);

    for(auto &it : m_variables) {
        TString type;
        switch(it.second.type()) {
            case MetaType::BOOLEAN: type = "bool"; break;
            case MetaType::INTEGER: type = "int"; break;
            case MetaType::FLOAT: type = "float"; break;
            default: break;
        }

        if(!type.isEmpty()) {
            pugi::xml_node variableElement = variablesElement.append_child(gVariable);
            variableElement.append_attribute(gName) = it.first.data();
            variableElement.append_attribute(gType) = type.data();
            variableElement.append_attribute(gValue) = it.second.toString().data();
        }
    }
}

StringList AnimationControllerGraph::nodeList() const {
    return m_nodeTypes;
}

Variant AnimationControllerGraph::data() const {
    VariantMap result;

    VariantList machine;
    // Pack states
    VariantList states;
    for(auto it : m_nodes) {
        BaseState *ptr = dynamic_cast<BaseState *>(it);
        if(ptr) {
            VariantList state;

            state.push_back(ptr->typeName());
            state.push_back(ptr->name());
            state.push_back(Engine::reference(ptr->clip()));
            state.push_back(ptr->loop());

            states.push_back(state);
        }
    }
    machine.push_back(states);
    // Pack variables
    VariantMap variables;
    for(auto &it : m_variables) {
        variables[it.first] = it.second;
    }
    machine.push_back(variables);
    // Pack transitions
    VariantList transitions;
    for(auto it : m_links) {
        StateLink *link = dynamic_cast<StateLink *>(it);
        if(link) {
            VariantList transition;
            transition.push_back(link->sender->name());
            transition.push_back(link->receiver->name());
            transition.push_back(link->duration());

            Variant condition(link->property(gCondition));
            if(condition.isValid()) {
                VariantList conditions;
                for(auto &it : condition.toList()) {
                    VariantMap map(it.toMap());

                    VariantList data;
                    data.push_back(map[gName]);
                    data.push_back(map[gType]);
                    data.push_back(map[gValue]);
                    conditions.push_back(data);
                }
                if(!conditions.empty()) {
                    transition.push_back(conditions);
                }
            }

            transitions.push_back(transition);
        }
    }
    machine.push_back(transitions);
    // Set initial state
    TString entry;
    for(const auto it : m_links) {
        if(it->sender == m_entryState) {
            entry = it->receiver->name();
            break;
        }
    }
    machine.push_back(entry);

    result[gMachine] = machine;
    return result;
}
