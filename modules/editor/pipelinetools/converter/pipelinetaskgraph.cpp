#include "pipelinetaskgraph.h"

#include <objectsystem.h>

#include <QUrl>
#include <QFileInfo>

PipelineTaskGraph::PipelineTaskGraph() {
    for(auto &it : Engine::factories()) {
        QUrl url(it.second.c_str());

        if(url.host() == "pipeline") {
            m_nodeTypes << url.fileName();
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

                            VariantList field;
                            field.push_back(l->sender->typeName());
                            field.push_back(l->receiver->typeName());
                            field.push_back(l->oport->m_var.toInt());
                            field.push_back(l->iport->m_var.toInt());

                            m_taskLinks.push_back(field);
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

void PipelineTaskGraph::onNodeUpdated() {

}

void PipelineTaskGraph::loadUserValues(GraphNode *node, const QVariantMap &values) {

}

void PipelineTaskGraph::saveUserValues(GraphNode *node, QVariantMap &values) const {

}

GraphNode *PipelineTaskGraph::nodeCreate(const QString &path, int &index) {
    PipelineNode *node = new PipelineNode();
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

GraphNode *PipelineTaskGraph::createRoot() {
    PipelineRootNode *result = new PipelineRootNode();
    result->setGraph(this);
    connect(result, &PipelineRootNode::graphUpdated, this, &PipelineTaskGraph::graphUpdated);

    return result;
}
