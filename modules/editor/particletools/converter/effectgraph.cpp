#include "effectgraph.h"

#include <QDirIterator>

#include <url.h>
#include <editor/projectsettings.h>

#include <resources/visualeffect.h>
#include <systems/resourcesystem.h>

#include <pugixml.hpp>

#include "effectrootnode.h"
#include "effectbuilder.h"
#include "modules/emitterstate.h"
#include "modules/effectmodule.h"
#include "modules/spritemodule.h"
#include "modules/meshmodule.h"
#include "modules/custommodule.h"

namespace {
    const char *gEmitters("Emitters");
};

EffectGraph::EffectGraph() :
        m_rootNode(nullptr) {

    m_version = EffectBuilder::version();

    GraphNode::registerClassFactory(Engine::resourceSystem());
    EffectRootNode::registerClassFactory(Engine::resourceSystem());
    EffectModule::registerClassFactory(Engine::resourceSystem());
    EmitterState::registerClassFactory(Engine::resourceSystem());
    SpriteParticle::registerClassFactory(Engine::resourceSystem());
    RenderableModule::registerClassFactory(Engine::resourceSystem());
    MeshParticle::registerClassFactory(Engine::resourceSystem());
    CustomModule::registerClassFactory(Engine::resourceSystem());

    m_nodeTypes.push_back("EmitterUpdate/EmitterState");

    scanForFunctions();

    m_nodeTypes.push_back("Render/SpriteParticle");
    m_nodeTypes.push_back("Render/MeshParticle");
}

void EffectGraph::scanForFunctions() {
    QStringList filter({"*.vfm"});

    StringList paths = {
        ":/modules",
        ProjectSettings::instance()->contentPath()
    };

    for(auto &path : paths) {
        QStringList files;

        QDirIterator it(path.data(), filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(it.hasNext()) {
            files << it.next();
        }

        for(auto &path : files) {
            QFile file(path);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                pugi::xml_document doc;
                if(doc.load_string(file.readAll().data()).status == pugi::status_ok) {
                    pugi::xml_node function = doc.document_element();

                    TString name(function.attribute("name").as_string());

                    m_nodeTypes.push_back(name);
                    m_exposedModules[Url(name).baseName()] = path.toStdString();
                }
            }
        }
    }
}

GraphNode *EffectGraph::nodeCreate(const TString &type, int &index) {
    if(type == "EffectEmitter") {
        GraphNode *node = Engine::objectCreate<EffectRootNode>();
        node->setGraph(this);

        if(index == -1) {
            index = m_nodes.size();
            m_nodes.push_back(node);
        } else {
            m_nodes.insert(std::next(m_nodes.begin(), index), node);
        }

        return node;
    }

    return nullptr;
}

void EffectGraph::onNodesLoaded() {
    m_rootNode = nullptr;

    for(auto it : m_nodes) {
        EffectRootNode *entry = dynamic_cast<EffectRootNode *>(it);
        if(entry) {
            m_rootNode = entry;
            break;
        }
    }

    if(m_rootNode == nullptr) {
        m_rootNode = new EffectRootNode;

        m_rootNode->setName("EffectEmitter");
        m_rootNode->setGraph(this);

        m_nodes.push_front(m_rootNode);
    }
}

StringList EffectGraph::nodeList() const {
    return m_nodeTypes;
}

GraphNode *EffectGraph::defaultNode() const {
    return m_rootNode;
}

TString EffectGraph::modulePath(const TString &name) {
    auto it = m_exposedModules.find(name);
    if(it != m_exposedModules.end()) {
        return it->second;
    }
    return TString();
}

StringList EffectGraph::modules() const {
    return m_nodeTypes;
}

VariantMap EffectGraph::data() const {
    VariantMap user;

    VariantList emitters;
    for(GraphNode *node : m_nodes) {
        EffectRootNode *rootNode = dynamic_cast<EffectRootNode *>(node);
        if(rootNode) {
            emitters.push_back(rootNode->saveData());
        }
    }
    user[gEmitters] = emitters;

    return user;
}

Variant EffectGraph::object() const {
    VariantList result;

    VariantList object;

    object.push_back(VisualEffect::metaClass()->name()); // type
    object.push_back(Engine::generateUUID()); // id
    object.push_back(0); // parent
    object.push_back(""); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}
