#include "pipelinetaskgraph.h"

#include "pipelineconverter.h"

#include <systems/resourcesystem.h>
#include <url.h>
#include <log.h>

namespace {
    const char *gRootNode("RootNode");
};

PipelineTaskGraph::PipelineTaskGraph() :
        m_rootNode(nullptr) {

    PipelineRootNode::registerClassFactory(Engine::resourceSystem());
    PipelineNode::registerClassFactory(Engine::resourceSystem());

    m_version = PipelineConverterSettings::version();

    for(auto &it : Engine::factories()) {
        Url url(it.second);

        if(url.host() == "Pipeline") {
            m_nodeTypes.push_back(url.baseName());
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
                                aDebug() << "Unable to find a link";
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

StringList PipelineTaskGraph::nodeList() const {
    return m_nodeTypes;
}

GraphNode *PipelineTaskGraph::fallbackRoot() {
    GraphNode *node = Engine::objectCreate<PipelineRootNode>("PipelineRootNode");
    node->setGraph(this);
    m_nodes.push_front(node);

    return node;
}

GraphNode *PipelineTaskGraph::nodeCreate(const TString &type, int &index) {
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

void PipelineTaskGraph::onNodesLoaded() {
    m_rootNode = nullptr;

    for(auto it : m_nodes) {
        PipelineRootNode *root = dynamic_cast<PipelineRootNode *>(it);
        if(root) {
            m_rootNode = root;
            break;
        }
    }
}
