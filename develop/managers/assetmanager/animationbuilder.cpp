#include "animationbuilder.h"

#include <QFile>
#include <QMetaClassInfo>

#include <bson.h>

#include <editor/graph/graphnode.h>

#include <resources/animationstatemachine.h>
#include <resources/animationclip.h>

#define ENTRY "Entry"
#define NAME "Name"
#define CLIP "Clip"
#define LOOP "Loop"

#define MACHINE "Machine"

#define FORMAT_VERSION 1

Vector2 EntryState::defaultSize() const {
    return Vector2(170.0f, 40.0f);
}

Vector4 EntryState::color() const {
    return Vector4(0.1f, 0.1f, 0.1f, 1.0f);
}

bool EntryState::isState() const {
    return true;
}


BaseState::BaseState() {
    m_path = Template("", MetaType::type<AnimationClip *>());
    m_loop = false;
}

Template BaseState::clip() const {
    return m_path;
}

void BaseState::setClip(const Template &path) {
    m_path.path = path.path;
    emit updated();
}

bool BaseState::loop() const {
    return m_loop;
}

void BaseState::setLoop(bool loop) {
    m_loop = loop;
    emit updated();
}


AnimationNodeGraph::AnimationNodeGraph() {
    m_entry = nullptr;

    qRegisterMetaType<BaseState*>("BaseState");

    m_functions << "BaseState";
}

void AnimationNodeGraph::load(const QString &path) {
    AbstractNodeGraph::load(path);

    m_entry = m_nodes.at(m_data[ENTRY].toInt());
    if(m_entry) {
        linkCreate(m_rootNode, nullptr, m_entry, nullptr);
    }
}

void AnimationNodeGraph::save(const QString &path) {
    m_data[ENTRY] = m_nodes.indexOf(m_entry);

    AbstractNodeGraph::save(path);
}

GraphNode *AnimationNodeGraph::createRoot() {
    EntryState *result = new EntryState;

    result->setObjectName(ENTRY);

    return result;
}

GraphNode *AnimationNodeGraph::nodeCreate(const QString &path, int &index) {
    BaseState *node = new BaseState();
    connect(node, &BaseState::updated, this, &AnimationNodeGraph::graphUpdated);
    node->setObjectName(path);
    node->setGraph(this);
    node->setType(qPrintable(path));
    if(index == -1) {
        index = m_nodes.size();
        m_nodes.push_back(node);
    } else {
        m_nodes.insert(index, node);
    }

    return node;
}

AnimationNodeGraph::Link *AnimationNodeGraph::linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) {
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

void AnimationNodeGraph::loadUserValues(GraphNode *node, const QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node);
    node->setObjectName(values[NAME].toString());

    Template tpl;
    tpl.path = values[CLIP].toString();
    tpl.type = ptr->clip().type;
    ptr->setClip(tpl);
    ptr->setLoop(values[LOOP].toBool());
}

void AnimationNodeGraph::saveUserValues(GraphNode *node, QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node);
    values[NAME] = node->objectName();
    values[CLIP] = ptr->clip().path;
    values[LOOP] = ptr->loop();
}

Variant AnimationNodeGraph::object() const {
    VariantList result;

    VariantList object;

    object.push_back(AnimationStateMachine::metaClass()->name()); // type
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(AnimationStateMachine::metaClass()->name()); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}

QStringList AnimationNodeGraph::nodeList() const {
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

Variant AnimationNodeGraph::data() const {
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

AnimationBuilderSettings::AnimationBuilderSettings() {
    setType(MetaType::type<AnimationStateMachine *>());
    setVersion(FORMAT_VERSION);
}

QString AnimationBuilderSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/machine.svg";
}

AssetConverter::ReturnCode AnimationBuilder::convertFile(AssetConverterSettings *settings) {
    m_model.load(settings->source());
    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( m_model.object() );
        file.write((const char *)&data[0], data.size());
        file.close();
        return Success;
    }
    return InternalError;
}

AssetConverterSettings *AnimationBuilder::createSettings() const {
    return new AnimationBuilderSettings();
}
