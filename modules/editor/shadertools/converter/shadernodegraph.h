#ifndef SHADERNODEGRAPH_H
#define SHADERNODEGRAPH_H

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

#include "rootnodes.h"

class RenderTarget;
class CommandBuffer;

class ShaderNodeGraph : public AbstractNodeGraph {
    Q_OBJECT

    enum Stage {
        Fragment,
        Vertex
    };

public:
    ShaderNodeGraph();
    ~ShaderNodeGraph() Q_DECL_OVERRIDE;

    VariantMap data(bool editor = false, ShaderRootNode *root = nullptr) const;

    bool buildGraph(GraphNode *node = nullptr);

    int addTexture(const QString &path, Vector4 &sub, int32_t flags = 0);

    void addUniform(const QString &name, uint8_t type, const QVariant &value);

    void addFunction(const QString &name, QString &code);

    QStringList nodeList() const Q_DECL_OVERRIDE;

    void load(const QString &path) Q_DECL_OVERRIDE;
    void save(const QString &path) Q_DECL_OVERRIDE;

    void loadGraph(const QVariantMap &data) Q_DECL_OVERRIDE;

    void setPreviewVisible(GraphNode *node, bool visible) Q_DECL_OVERRIDE;
    void updatePreviews(CommandBuffer &buffer);

private slots:
    void onNodeUpdated();

private:
    void markDirty(GraphNode *node);
    Texture *preview(GraphNode *node) Q_DECL_OVERRIDE;

    QString buildFrom(GraphNode *node, Stage stage);

    void loadUserValues(GraphNode *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(GraphNode *node, QVariantMap &values) Q_DECL_OVERRIDE;

    GraphNode *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;
    GraphNode *createRoot() Q_DECL_OVERRIDE;
    void nodeDelete(GraphNode *node) Q_DECL_OVERRIDE;

    Variant compile(int32_t rhi, const QString &source, const string &define, int stage) const;

    void cleanup();

    void setPragma(const string &key, const string &value);

    void scanForCustomFunctions();

private:
    struct MaterialInput {
        MaterialInput(QString name, QVariant value, bool vertex = false) :
            m_name(name),
            m_value(value),
            m_vertex(vertex) {

        }

        QString m_name;

        QVariant m_value;

        bool m_vertex;
    };

    struct UniformData {
        QString name;

        uint32_t type;

        size_t count;

        QVariant value;
    };

    struct PreviewData {
        Material *material = nullptr;
        MaterialInstance *instance = nullptr;

        Texture *texture = nullptr;

        RenderTarget *target = nullptr;

        bool isDirty = true;

        bool isVisible = false;
    };

private:
    QStringList m_nodeTypes;

    list<UniformData> m_uniforms;

    QList<QPair<QString, int32_t>> m_textures;

    map<QString, QString> m_functions;

    map<QString, QString> m_exposedFunctions;

    map<GraphNode *, PreviewData> m_previews;

    map<string, string> m_pragmas;

    list<MaterialInput> m_inputs;

    ShaderRootNode m_previewSettings;

};

#endif // SHADERNODEGRAPH_H
