#include "animationcontrollergraph.h"

#include <QMetaClassInfo>

#include "basestate.h"
#include "animationbuilder.h"
#include <resources/animationstatemachine.h>

namespace {
    const char *gEntry("Entry");
    const char *gName("Name");
    const char *gClip("Clip");
    const char *gLoop("Loop");

    const char *gMachine("Machine");

    const char *gBaseState("BaseState");

    const char *gUser("user");
};

AnimationControllerGraph::AnimationControllerGraph() {
    m_version = AnimationControllerBuilder::version();

    m_entry = nullptr;

    qRegisterMetaType<BaseState*>(gBaseState);

    m_functions << gBaseState;
}

void AnimationControllerGraph::loadGraphV0(const QVariantMap &data) {
    AbstractNodeGraph::loadGraphV0(data);

    int32_t entry = data[gEntry].toInt();
    if(entry > -1) {
        m_entry = m_nodes.at(entry);
    }

    if(m_entry) {
        linkCreate(m_rootNode, nullptr, m_entry, nullptr);
    }
}

void AnimationControllerGraph::loadGraphV11(const QDomElement &parent) {
    AbstractNodeGraph::loadGraphV11(parent);

    if(parent.tagName() == gUser) {
        int32_t entry = parent.attribute(gEntry).toInt();
        if(entry > -1) {
            m_entry = m_nodes.at(entry);
        }

        if(m_entry) {
            linkCreate(m_rootNode, nullptr, m_entry, nullptr);
        }
    }
}

void AnimationControllerGraph::saveGraph(QDomElement parent, QDomDocument xml) const {
    AbstractNodeGraph::saveGraph(parent, xml);

    QDomElement user = xml.createElement(gUser);

    user.setAttribute(gEntry, QString::number(m_nodes.indexOf(m_entry)));

    parent.appendChild(user);
}

GraphNode *AnimationControllerGraph::createRoot() {
    EntryState *result = new EntryState;

    result->setObjectName(gEntry);
    result->setGraph(this);

    return result;
}

GraphNode *AnimationControllerGraph::nodeCreate(const QString &path, int &index) {
    BaseState *node = new BaseState();
    connect(node, &BaseState::updated, this, &AnimationControllerGraph::graphUpdated);
    node->setObjectName(path);
    node->setGraph(this);
    node->setTypeName(qPrintable(path));
    if(index == -1) {
        index = m_nodes.size();
        m_nodes.push_back(node);
    } else {
        m_nodes.insert(index, node);
    }

    return node;
}

AnimationControllerGraph::Link *AnimationControllerGraph::linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) {
    if(receiver == m_rootNode) {
        return nullptr;
    }
    if(sender == m_rootNode) {
        auto it = m_links.begin();
        while(it != m_links.end()) {
            Link *link = *it;
            if(link->sender == sender) {
                it  = m_links.erase(it);
                delete link;
            } else {
                ++it;
            }
        }
    }
    return AbstractNodeGraph::linkCreate(sender, oport, receiver, iport);
}

void AnimationControllerGraph::loadUserValues(GraphNode *node, const QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node);
    node->setObjectName(values[gName].toString());

    Template tpl;
    tpl.path = values[gClip].toString();
    tpl.type = ptr->clip().type;
    ptr->setClip(tpl);
    ptr->setLoop(values[gLoop].toBool());
}

void AnimationControllerGraph::saveUserValues(GraphNode *node, QVariantMap &values) const {
    BaseState *ptr = reinterpret_cast<BaseState *>(node);
    values[gName] = node->objectName();
    values[gClip] = ptr->clip().path;
    values[gLoop] = ptr->loop();
}

Variant AnimationControllerGraph::object() const {
    VariantList result;

    VariantList object;

    object.push_back(AnimationStateMachine::metaClass()->name()); // type
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(""); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}

QStringList AnimationControllerGraph::nodeList() const {
    QStringList result;
    for(auto &it : m_functions) {
        const int type = QMetaType::type( qPrintable(it) );
        const QMetaObject *meta = QMetaType::metaObjectForType(type);
        if(meta) {
            int index = meta->indexOfClassInfo("Group");
            if(index != -1) {
                result << QString(meta->classInfo(index).value()) + "/" + it;
            }
        }
    }

    return result;
}

Variant AnimationControllerGraph::data() const {
    VariantMap result;

    VariantList machine;
    // Pack states
    VariantList states;
    for(auto it : m_nodes) {
        if(it != m_rootNode) {
            BaseState *ptr = reinterpret_cast<BaseState *>(it);

            VariantList state;
            state.push_back(0); // Default state
            state.push_back(qPrintable(it->objectName())); // Name of state
            state.push_back(qPrintable(ptr->clip().path));
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
        transition.push_back(qPrintable(it->sender->objectName()));
        transition.push_back(qPrintable(it->receiver->objectName()));

        transitions.push_back(transition);
    }
    machine.push_back(transitions);
    // Set initial state
    QString entry;
    for(const auto it : m_links) {
        if(it->sender == m_rootNode) {
            entry = it->receiver->objectName();
            break;
        }
    }
    machine.push_back(qPrintable(entry));

    result[gMachine] = machine;
    return result;
}
