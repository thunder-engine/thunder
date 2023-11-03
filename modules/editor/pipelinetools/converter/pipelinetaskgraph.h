#ifndef PIPELINETASKGRAPH_H
#define PIPELINETASKGRAPH_H

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

#include <pipelinetask.h>
#include <metaobject.h>

class RenderTarget;
class CommandBuffer;

const static Vector4 gColor(0.6f, 0.6f, 0.6f, 1.0f);

class PipelineRootNode : public GraphNode {
    Q_OBJECT

public:
    PipelineRootNode() {
        NodePort port(this, false, QMetaType::Bool, 0, "", gColor, 0);
        port.m_call = true;
        m_ports.push_back(port);
        m_ports.push_back(NodePort(this, false, QMetaType::Int, 1, "input", gColor, 0));
    }

    Vector2 defaultSize() const override { return Vector2(150.0f, 30.0f); }
    Vector4 color() const override { return Vector4(0.141f, 0.384f, 0.514f, 1.0f); }

signals:
    void graphUpdated();

};

class PipelineNode : public GraphNode {
    Q_OBJECT

public:
    PipelineNode() :
            m_task(nullptr) {

        NodePort in(this, false, QMetaType::Bool, 0, "", gColor, 0);
        in.m_call = true;
        m_ports.push_back(in);

        NodePort out(this, true, QMetaType::Bool, 1, "", gColor, 0);
        out.m_call = true;
        m_ports.push_back(out);

    }

    void setTypeName(const std::string &name) override {
        GraphNode::setTypeName(name);

        m_task = static_cast<PipelineTask *>(Engine::objectCreate(name));
        if(m_task) {
            int index = m_ports.size();
            for(int i = 0; i < m_task->outputCount(); i++) {
                m_ports.push_back(NodePort(this, true, QMetaType::Int, index + i, m_task->outputName(i), gColor, i));
            }
            index += m_task->outputCount();

            for(int i = 0; i < m_task->inputCount(); i++) {
                m_ports.push_back(NodePort(this, false, QMetaType::Int, index + i, m_task->inputName(i), gColor, i));
            }
        }
    }

    Vector2 defaultSize() const override { return Vector2(150.0f, 30.0f); }
    Vector4 color() const override { return Vector4(0.141f, 0.384f, 0.514f, 1.0f); }

signals:
    void graphUpdated();

private:
    PipelineTask *m_task;

};

class PipelineTaskGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    PipelineTaskGraph();

    VariantMap data() const;

    bool buildGraph();

    QStringList nodeList() const Q_DECL_OVERRIDE;

private slots:
    void onNodeUpdated();

private:
    void loadUserValues(GraphNode *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(GraphNode *node, QVariantMap &values) Q_DECL_OVERRIDE;

    GraphNode *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;
    GraphNode *createRoot() Q_DECL_OVERRIDE;

private:
    QStringList m_nodeTypes;

    VariantList m_tasks;

    VariantList m_taskLinks;

};

#endif // PIPELINETASKGRAPH_H
