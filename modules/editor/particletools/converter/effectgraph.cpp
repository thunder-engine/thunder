#include "effectgraph.h"

#include <QDirIterator>

#include <editor/projectsettings.h>

#include <resources/visualeffect.h>

#include "effectrootnode.h"
#include "effectbuilder.h"

namespace {
    const char *gEmitters("Emitters");
};

EffectGraph::EffectGraph() :
        m_rootNode(nullptr) {

    m_version = EffectBuilder::version();

    scanForFunctions();
}

void EffectGraph::scanForFunctions() {
    QStringList filter({"*.vfm"});

    QStringList paths = {
        ":/modules",
        ProjectSettings::instance()->contentPath()
    };

    for(auto &path : paths) {
        QStringList files;

        QDirIterator it(path, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(it.hasNext()) {
            files << it.next();
        }

        for(auto &path : files) {
            QFile file(path);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QDomDocument doc;
                if(doc.setContent(&file)) {
                    QDomElement function = doc.documentElement();

                    QString name(function.attribute("name"));

                    m_nodeTypes << name;
                    m_exposedModules[QFileInfo(name).baseName()] = path;
                }
            }
        }
    }
}

GraphNode *EffectGraph::nodeCreate(const QString &path, int &index) {
    if(path == "EffectRootNode") {
        GraphNode *node = new EffectRootNode;
        node->setGraph(this);
        connect(node, &EffectRootNode::updated, this, &EffectGraph::effectUpdated);

        if(index == -1) {
            index = m_nodes.size();
            m_nodes.push_back(node);
        } else {
            m_nodes.insert(index, node);
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

        m_rootNode->setObjectName("NewEffectEmitter");
        m_rootNode->setGraph(this);
        connect(m_rootNode, &EffectRootNode::updated, this, &EffectGraph::effectUpdated);

        m_nodes.push_front(m_rootNode);
    }
}

QStringList EffectGraph::nodeList() const {
    return m_nodeTypes;
}

EffectRootNode *EffectGraph::rootNode() {
    return m_rootNode;
}

QString EffectGraph::modulePath(QString name) {
    auto it = m_exposedModules.find(name);
    if(it != m_exposedModules.end()) {
        return it->second;
    }
    return QString();
}

QStringList EffectGraph::modules() const {
    return m_nodeTypes;
}

void EffectGraph::onAddModule(const QString &name) {
    m_rootNode->addModule(m_exposedModules[name].toStdString());
    emit moduleChanged();
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
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(""); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}
