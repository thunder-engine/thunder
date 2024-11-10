#include "pipelinetaskgraph.h"

#include "pipelineconverter.h"

#include <objectsystem.h>
#include <uri.h>

namespace {
    const char *gRootNode("RootNode");
};

PipelineTaskGraph::PipelineTaskGraph() :
        m_rootNode(nullptr) {
    m_version = PipelineConverterSettings::version();

    for(auto &it : Engine::factories()) {
        Uri uri(it.second);

        if(uri.host() == "pipeline") {
            m_nodeTypes << uri.baseName().c_str();
        }
    }
}

VariantMap PipelineTaskGraph::data() const {
    VariantMap result;

    result["Tasks"] = m_tasks;
    result["Links"] = m_taskLinks;

    return result;
}

bool PipelineTaskGraph::buildGraph() {
    GraphNode *node = m_rootNode;

    m_tasks.clear();
    m_taskLinks.clear();

    while(node != nullptr) {
        GraphNode *next = nullptr;
        for(const auto &it : node->ports()) {
            if(!it.m_out && it.m_call) {
                auto link = findLink(node, &it);

                if(link) {
                    next = link->sender;
                    m_tasks.push_back(next->typeName());

                    for(auto &port : link->sender->ports()) {
                        if(!port.m_out && !port.m_call) {
                            auto l = findLink(link->sender, &port);

                            if(l) {
                                VariantList field;
                                field.push_back(l->sender->typeName());
                                field.push_back(l->receiver->typeName());
                                field.push_back(l->oport->m_var.toInt());
                                field.push_back(l->iport->m_var.toInt());

                                m_taskLinks.push_back(field);
                            } else {
                                qDebug() << "Unable to find a link";
                            }
                        }
                    }

                    break;
                }
            }
        }

        node = next;
    }

    m_tasks.reverse();
    m_taskLinks.reverse();

    return true;
}

QStringList PipelineTaskGraph::nodeList() const {
    return m_nodeTypes;
}

GraphNode *PipelineTaskGraph::nodeCreate(const QString &path, int &index) {
    GraphNode *node = nullptr;
    if(path == gRootNode) {
        node = new PipelineRootNode;
    } else {
        node = new PipelineNode;
    }

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

void PipelineTaskGraph::onNodesLoaded() {
    m_rootNode = nullptr;

    for(auto it : m_nodes) {
        PipelineRootNode *root = dynamic_cast<PipelineRootNode *>(it);
        if(root) {
            m_rootNode = root;
            break;
        }
    }

    if(m_rootNode == nullptr) {
        m_rootNode = new PipelineRootNode();
        m_rootNode->setGraph(this);
        m_rootNode->setTypeName(gRootNode);
        connect(m_rootNode, &PipelineRootNode::graphUpdated, this, &PipelineTaskGraph::graphUpdated);

        m_nodes.push_front(m_rootNode);
    }
}
