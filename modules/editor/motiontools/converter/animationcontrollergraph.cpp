#include "animationcontrollergraph.h"

#include "basestate.h"
#include "animationbuilder.h"

#include <resources/animationstatemachine.h>
#include <pugixml.hpp>

namespace {
    const char *gMachine("Machine");

};

AnimationControllerGraph::AnimationControllerGraph() :
        m_entryState(nullptr) {
    m_version = AnimationControllerBuilder::version();

    EntryState::registerClassFactory(Engine::resourceSystem());
    BaseState::registerClassFactory(Engine::resourceSystem());

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

AnimationControllerGraph::Link *AnimationControllerGraph::linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) {
    if(receiver == m_entryState) {
        return nullptr;
    }
    if(sender == m_entryState) {
        auto it = m_links.begin();
        while(it != m_links.end()) {
            Link *link = *it;
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

Variant AnimationControllerGraph::object() const {
    VariantList result;

    VariantList object;

    object.push_back(AnimationStateMachine::metaClass()->name()); // type
    object.push_back(Engine::generateUUID()); // id
    object.push_back(0); // parent
    object.push_back(""); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(data()); // user data

    result.push_back(object);

    return result;
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

            state.push_back("BaseState"); // Default state
            state.push_back(it->name()); // Name of state
            state.push_back(Engine::reference(ptr->clip()));
            state.push_back(ptr->loop());

            states.push_back(state);
        }
    }
    machine.push_back(states);
    // Pack variables
    VariantList variables;

    machine.push_back(variables);
    // Pack transitions
    VariantList transitions;
    for(auto it : m_links) {
        VariantList transition;
        transition.push_back(it->sender->name());
        transition.push_back(it->receiver->name());

        transitions.push_back(transition);
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
