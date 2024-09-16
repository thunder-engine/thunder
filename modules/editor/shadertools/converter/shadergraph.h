#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

#include "rootnode.h"

class RenderTarget;
class CommandBuffer;

enum OldBlendType {
    Opaque,
    Additive,
    Translucent
};

class ShaderGraph : public AbstractNodeGraph {
    Q_OBJECT

    enum Stage {
        Fragment,
        Vertex
    };

public:
    ShaderGraph();
    ~ShaderGraph() Q_DECL_OVERRIDE;

    VariantMap data(bool editor = false, ShaderRootNode *root = nullptr);

    bool buildGraph(GraphNode *node = nullptr);

    int addTexture(const QString &path, Vector4 &sub, int32_t flags = 0);

    void addUniform(const QString &name, uint8_t type, const QVariant &value);

    void addFunction(const QString &name, QString &code);

    QStringList nodeList() const override;

    void loadGraphV0(const QVariantMap &data) override;
    void loadGraphV11(const QDomElement &parent) override;
    void saveGraph(QDomElement parent, QDomDocument xml) const override;

    void setPreviewVisible(GraphNode *node, bool visible);
    Texture *preview(GraphNode *node);

    void updatePreviews(CommandBuffer &buffer);

private slots:
    void onNodeUpdated();

private:
    void markDirty(GraphNode *node);

    QString buildFrom(GraphNode *node, Stage stage);

    void loadUserValues(GraphNode *node, const QVariantMap &values) override;
    void saveUserValues(GraphNode *node, QVariantMap &values) const override;

    GraphNode *nodeCreate(const QString &path, int &index) override;
    GraphNode *createRoot() override;
    void nodeDelete(GraphNode *node) override;

    Variant compile(int32_t rhi, const QString &source, const std::string &define, int stage) const;

    void cleanup();

    void setPragma(const std::string &key, const std::string &value);

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

    std::list<UniformData> m_uniforms;

    QList<QPair<QString, int32_t>> m_textures;

    std::map<QString, QString> m_functions;

    std::map<QString, QString> m_exposedFunctions;

    std::map<GraphNode *, PreviewData> m_previews;

    std::map<std::string, std::string> m_pragmas;

    std::list<MaterialInput> m_inputs;

    ShaderRootNode m_previewSettings;

};

#endif // SHADERGRAPH_H
