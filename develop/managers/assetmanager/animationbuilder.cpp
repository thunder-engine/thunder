#include "animationbuilder.h"

#include <QVariant>
#include <QDir>
#include <QJsonDocument>

#include <components/animator.h>
#include <resources/animationstatemachine.h>

#include <engine.h>

#include <QFile>

#include <json.h>
#include <bson.h>

#include "assetmanager.h"
#include "projectmanager.h"

#include "functionmodel.h"

#define ENTRY "Entry"
#define NAME "Name"
#define CLIP "Clip"
#define LOOP "Loop"

#define MACHINE "Machine"

AnimationBuilderSettings::AnimationBuilderSettings() {
    setType(MetaType::type<AnimationStateMachine *>());
}

AnimationBuilder::AnimationBuilder() {
    m_pEntry = nullptr;
    m_pRootNode->name = "Entry";

    qRegisterMetaType<BaseState*>("BaseState");

    m_Functions << "BaseState";
}

uint8_t AnimationBuilder::convertFile(AssetConverterSettings *settings) {
    load(settings->source());
    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data  = Bson::save( object() );
        file.write((const char *)&data[0], data.size());
        file.close();
        return 0;
    }
    return 1;
}

AssetConverterSettings *AnimationBuilder::createSettings() const {
    return new AnimationBuilderSettings();
}

void AnimationBuilder::load(const QString &path) {
    AbstractSchemeModel::load(path);

    m_pEntry = m_Nodes.at(m_Data[ENTRY].toInt());
    if(m_pEntry) {
        linkCreate(m_pRootNode, nullptr, m_pEntry, nullptr);
    }
}

void AnimationBuilder::save(const QString &path) {
    m_Data[ENTRY] = m_Nodes.indexOf(m_pEntry);

    AbstractSchemeModel::save(path);
}

AbstractSchemeModel::Node *AnimationBuilder::nodeCreate(const QString &path, int &index) {
    Node *node = new Node;
    node->root = false;
    node->name = path;

    BaseState *data = new BaseState(node);
    connect(data, SIGNAL(updated()), this, SIGNAL(schemeUpdated()));
    node->ptr = data;

    if(index == -1) {
        index = m_Nodes.size();
        m_Nodes.push_back(node);
    } else {
        m_Nodes.insert(index, node);
    }

    return node;
}

AbstractSchemeModel::Link *AnimationBuilder::linkCreate(Node *sender, Port *oport, Node *receiver, Port *iport) {
    if(receiver == m_pRootNode) {
        return nullptr;
    }
    if(sender == m_pRootNode) {
        auto it = m_Links.begin();
        while(it != m_Links.end()) {
            Link *link = *it;
            if(link->sender == sender) {
                it  = m_Links.erase(it);
                delete link;
            } else {
                ++it;
            }
        }
    }
    return AbstractSchemeModel::linkCreate(sender, oport, receiver, iport);
}

void AnimationBuilder::loadUserValues(AbstractSchemeModel::Node *node, const QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node->ptr);
    node->name = values[NAME].toString();

    Template tpl;
    tpl.path = values[CLIP].toString();
    tpl.type = ptr->clip().type;
    ptr->setClip(tpl);
    ptr->setLoop(values[LOOP].toBool());
}

void AnimationBuilder::saveUserValues(Node *node, QVariantMap &values) {
    BaseState *ptr = reinterpret_cast<BaseState *>(node->ptr);
    values[NAME] = node->name;
    values[CLIP] = ptr->clip().path;
    values[LOOP] = ptr->loop();
}

QAbstractItemModel *AnimationBuilder::components() const {
    return new FunctionModel(m_Functions);
}

Variant AnimationBuilder::object() const {
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

Variant AnimationBuilder::data() const {
    VariantMap result;

    VariantList machine;
    // Pack states
    VariantList states;
    for(auto it : m_Nodes) {
        if(it != m_pRootNode) {
            BaseState *ptr = reinterpret_cast<BaseState *>(it->ptr);

            VariantList state;
            state.push_back(0); // Default state
            state.push_back(qPrintable(it->name)); // Name of state
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
    for(auto it : m_Links) {
        VariantList transition;
        transition.push_back(qPrintable(it->sender->name));
        transition.push_back(qPrintable(it->receiver->name));

        transitions.push_back(transition);
    }
    machine.push_back(transitions);
    // Set initial state
    QString entry;
    for(const auto it : m_Links) {
        if(it->sender == m_pRootNode) {
            entry = it->receiver->name;
            break;
        }
    }
    machine.push_back(qPrintable(entry));

    result[MACHINE] = machine;
    return result;
}
