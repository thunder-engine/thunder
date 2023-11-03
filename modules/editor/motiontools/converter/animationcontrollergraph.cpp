#include "animationcontrollergraph.h"

#include <QMetaClassInfo>

#include "basestate.h"
#include <resources/animationstatemachine.h>

#define ENTRY "Entry"
#define NAME "Name"
#define CLIP "Clip"
#define LOOP "Loop"

#define MACHINE "Machine"

AnimationControllerGraph::AnimationControllerGraph() {
    m_entry = nullptr;

    qRegisterMetaType<BaseState*>("BaseState");

    m_functions << "BaseState";
}

void AnimationControllerGraph::load(const QString &path) {
    AbstractNodeGraph::load(path);

    if(m_entry) {
        linkCreate(m_rootNode, nullptr, m_entry, nullptr);
    }
}

void AnimationControllerGraph::loadGraph(const QVariantMap &data) {
    AbstractNodeGraph::loadGraph(data);

    int32_t entry = m_data[ENTRY].toInt();
    if(entry > -1) {
        m_entry = m_nodes.at(entry);
    }
}

void AnimationControllerGraph::save(const QString &path) {
    m_data[ENTRY] = m_nodes.indexOf(m_entry);

    AbstractNodeGraph::save(path);
}

GraphNode *AnimationControllerGraph::createRoot() {
    EntryState *result = new EntryState;

    result->setObjectName(ENTRY);
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
    node->setObjectName(values[NAME].toString());

    Template tpl;
    tpl.path = values[CLIP].toString();
    tpl.type = ptr->clip().type;
    ptr->setClip(tpl);
    ptr->setLoop(values[LOOP].toBool());
}

void AnimationControllerGraph::saveUserValues(GraphNode *node, QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node);
    values[NAME] = node->objectName();
    values[CLIP] = ptr->clip().path;
    values[LOOP] = ptr->loop();
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

    result[MACHINE] = machine;
    return result;
}
