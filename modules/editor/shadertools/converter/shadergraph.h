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
    ~ShaderGraph() override;

    VariantMap data(bool editor = false, ShaderRootNode *root = nullptr);

    bool buildGraph(GraphNode *node = nullptr);

    int addTexture(const String &path, Vector4 &sub, int32_t flags = 0);

    void addUniform(const String &name, uint8_t type, const Variant &value);

    void addFunction(const String &name, String &code);

    StringList nodeList() const override;

    void onNodesLoaded() override;

    void setPreviewVisible(GraphNode *node, bool visible);
    Texture *preview(GraphNode *node);

    void updatePreviews(CommandBuffer &buffer);

private slots:
    void onNodeUpdated();

private:
    GraphNode *fallbackRoot() override;

    GraphNode *defaultNode() const override;

    void markDirty(GraphNode *node);

    QString buildFrom(GraphNode *node, Stage stage);

    GraphNode *nodeCreate(const String &type, int &index) override;

    void nodeDelete(GraphNode *node) override;

    Variant compile(int32_t rhi, const QString &source, const String &define, int stage) const;

    void cleanup();

    void setPragma(const String &key, const String &value);

    void scanForCustomFunctions();

private:
    struct MaterialInput {
        MaterialInput(String name, Variant value, bool vertex = false) :
                m_name(name),
                m_value(value),
                m_vertex(vertex) {

        }

        String m_name;

        Variant m_value;

        bool m_vertex;
    };

    struct UniformData {
        String name;

        uint32_t type;

        size_t count;

        Variant value;
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
    StringList m_nodeTypes;

    std::list<UniformData> m_uniforms;

    std::list<std::pair<String, int32_t>> m_textures;

    std::map<String, String> m_functions;

    std::map<String, String> m_exposedFunctions;

    std::map<String, String> m_pragmas;

    std::map<GraphNode *, PreviewData> m_previews;

    std::list<MaterialInput> m_inputs;

    ShaderRootNode m_previewSettings;

    ShaderRootNode *m_rootNode;

};

#endif // SHADERGRAPH_H
