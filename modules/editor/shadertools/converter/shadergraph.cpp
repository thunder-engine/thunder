#include "shadergraph.h"

#include <QDirIterator>

#include <algorithm>

#include <commandbuffer.h>
#include <pipelinecontext.h>
#include <resources/rendertarget.h>

#include <editor/graph/nodegroup.h>
#include <editor/projectsettings.h>
#include <editor/nativecodebuilder.h>

#include <systems/resourcesystem.h>
#include <os/uuid.h>
#include <url.h>

#include "functions/camera.h"
#include "functions/constvalue.h"
#include "functions/imageeffects.h"
#include "functions/coordinates.h"
#include "functions/materialparam.h"
#include "functions/mathoperator.h"
#include "functions/matrixoperations.h"
#include "functions/surface.h"
#include "functions/texturesample.h"
#include "functions/time.h"
#include "functions/trigonometry.h"
#include "functions/logicoperator.h"
#include "functions/vectoroperator.h"
#include "functions/customfunction.h"

#include "shaderbuilder.h"

namespace {
    const char *gUser("user");
    const char *gValue("value");

    const char *gType("type");

    const char *gModel("model");
    const char *gSide("side");
    const char *gWireFrame("wireframe");

    const char *gDepthWrite("depthwrite");

    const char *gBlend("blend");
    const char *gDepth("depth");
    const char *gStencil("stencil");
};

std::map<uint32_t, Vector4> ShaderNode::m_portColors = {
    { MetaType::INVALID, Vector4(0.6f, 0.6f, 0.6f, 1.0f) },
    { MetaType::INTEGER, Vector4(0.22f, 0.46, 0.11f, 1.0f) },
    { MetaType::FLOAT,   Vector4(0.16f, 0.52f, 0.80f, 1.0f) },
    { MetaType::VECTOR2, Vector4(0.95f, 0.26f, 0.21f, 1.0f) },
    { MetaType::VECTOR3, Vector4(0.41f, 0.19f, 0.62f, 1.0f) },
    { MetaType::VECTOR4, Vector4(0.94f, 0.76f, 0.20f, 1.0f) },
    { MetaType::MATRIX3, Vector4(0.5f, 0.93f, 0.44f, 1.0f) },
    { MetaType::MATRIX4, Vector4(0.5f, 0.93f, 0.44f, 1.0f) },
    { MetaType::STRING,  Vector4(0.93f, 0.5f, 0.07f, 1.0f) },
};

StringList ShaderGraph::m_nodeTypes;
std::map<TString, TString> ShaderGraph::m_exposedFunctions;

ShaderGraph::ShaderGraph() :
        m_rootNode(nullptr) {
    m_version = ShaderBuilder::version();

    if(m_nodeTypes.empty()) {
        GraphNode::registerClassFactory(&Engine::instance());

        ShaderRootNode::registerClassFactory(&Engine::instance());

        scanForCustomFunctions();

        // Base
        ShaderNode::registerClassFactory(&Engine::instance());

        // Constants
        ConstColor::registerClassFactory(&Engine::instance());
        ConstPi::registerClassFactory(&Engine::instance());
        ConstEuler::registerClassFactory(&Engine::instance());
        ConstGoldenRatio::registerClassFactory(&Engine::instance());
        ConstFloat::registerClassFactory(&Engine::instance());
        ConstInt::registerClassFactory(&Engine::instance());
        ConstVector2::registerClassFactory(&Engine::instance());
        ConstVector3::registerClassFactory(&Engine::instance());
        ConstVector4::registerClassFactory(&Engine::instance());
        ConstMatrix3::registerClassFactory(&Engine::instance());
        ConstMatrix4::registerClassFactory(&Engine::instance());

        // ImageEffects
        Desaturate::registerClassFactory(&Engine::instance());

        // Camera
        CameraPosition::registerClassFactory(&Engine::instance());
        CameraDirection::registerClassFactory(&Engine::instance());
        ScreenSize::registerClassFactory(&Engine::instance());
        ScreenPosition::registerClassFactory(&Engine::instance());
        ProjectionMatrix::registerClassFactory(&Engine::instance());
        ExtractPosition::registerClassFactory(&Engine::instance());

        // Coordinates
        TexCoord::registerClassFactory(&Engine::instance());
        ProjectionCoord::registerClassFactory(&Engine::instance());
        CoordPanner::registerClassFactory(&Engine::instance());

        // Parameters
        ParamFloat::registerClassFactory(&Engine::instance());
        ParamVector::registerClassFactory(&Engine::instance());

        // Texture
        TextureFunction::registerClassFactory(&Engine::instance());

        TextureObject::registerClassFactory(&Engine::instance());
        TextureSample::registerClassFactory(&Engine::instance());
        RenderTargetSample::registerClassFactory(&Engine::instance());
        TextureSampleCube::registerClassFactory(&Engine::instance());

        // Logic Operators
        If::registerClassFactory(&Engine::instance());
        Compare::registerClassFactory(&Engine::instance());

        // Math Operations
        Abs::registerClassFactory(&Engine::instance());
        Add::registerClassFactory(&Engine::instance());
        Ceil::registerClassFactory(&Engine::instance());
        Clamp::registerClassFactory(&Engine::instance());
        DDX::registerClassFactory(&Engine::instance());
        DDY::registerClassFactory(&Engine::instance());
        Divide::registerClassFactory(&Engine::instance());
        Exp::registerClassFactory(&Engine::instance());
        Exp2::registerClassFactory(&Engine::instance());
        Floor::registerClassFactory(&Engine::instance());
        Fract::registerClassFactory(&Engine::instance());
        FWidth::registerClassFactory(&Engine::instance());
        Mix::registerClassFactory(&Engine::instance());
        Logarithm::registerClassFactory(&Engine::instance());
        Logarithm10::registerClassFactory(&Engine::instance());
        Logarithm2::registerClassFactory(&Engine::instance());
        Max::registerClassFactory(&Engine::instance());
        Min::registerClassFactory(&Engine::instance());
        Multiply::registerClassFactory(&Engine::instance());
        Power::registerClassFactory(&Engine::instance());
        Round::registerClassFactory(&Engine::instance());
        Sign::registerClassFactory(&Engine::instance());
        Smoothstep::registerClassFactory(&Engine::instance());
        SquareRoot::registerClassFactory(&Engine::instance());
        Step::registerClassFactory(&Engine::instance());

        MathOperation::registerClassFactory(&Engine::instance());

        Subtraction::registerClassFactory(&Engine::instance());
        Truncate::registerClassFactory(&Engine::instance());
        InverseLerp::registerClassFactory(&Engine::instance());
        Fmod::registerClassFactory(&Engine::instance());
        Negate::registerClassFactory(&Engine::instance());
        Saturate::registerClassFactory(&Engine::instance());
        Scale::registerClassFactory(&Engine::instance());
        ScaleAndOffset::registerClassFactory(&Engine::instance());
        OneMinus::registerClassFactory(&Engine::instance());
        Remainder::registerClassFactory(&Engine::instance());
        RSqrt::registerClassFactory(&Engine::instance());
        TriangleWave::registerClassFactory(&Engine::instance());
        SquareWave::registerClassFactory(&Engine::instance());
        SawtoothWave::registerClassFactory(&Engine::instance());

        // Matrix operations
        MatrixOperation::registerClassFactory(&Engine::instance());

        Determinant::registerClassFactory(&Engine::instance());
        Inverse::registerClassFactory(&Engine::instance());
        Transpose::registerClassFactory(&Engine::instance());
        MakeMatrix::registerClassFactory(&Engine::instance());

        // Surface
        Fresnel::registerClassFactory(&Engine::instance());
        SurfaceDepth::registerClassFactory(&Engine::instance());
        WorldBitangent::registerClassFactory(&Engine::instance());
        WorldNormal::registerClassFactory(&Engine::instance());
        WorldPosition::registerClassFactory(&Engine::instance());
        WorldTangent::registerClassFactory(&Engine::instance());

        // Trigonometry operators
        ArcCosine::registerClassFactory(&Engine::instance());
        ArcSine::registerClassFactory(&Engine::instance());
        ArcTangent::registerClassFactory(&Engine::instance());
        ArcTangent2::registerClassFactory(&Engine::instance());
        Cosine::registerClassFactory(&Engine::instance());
        CosineHyperbolic::registerClassFactory(&Engine::instance());
        Degrees::registerClassFactory(&Engine::instance());
        Radians::registerClassFactory(&Engine::instance());
        Sine::registerClassFactory(&Engine::instance());
        SineHyperbolic::registerClassFactory(&Engine::instance());
        Tangent::registerClassFactory(&Engine::instance());
        TangentHyperbolic::registerClassFactory(&Engine::instance());

        // Time
        CosTime::registerClassFactory(&Engine::instance());
        DeltaTime::registerClassFactory(&Engine::instance());
        SinTime::registerClassFactory(&Engine::instance());
        Time::registerClassFactory(&Engine::instance());

        // Vector Operators
        Append::registerClassFactory(&Engine::instance());
        CrossProduct::registerClassFactory(&Engine::instance());
        Distance::registerClassFactory(&Engine::instance());
        DotProduct::registerClassFactory(&Engine::instance());
        Length::registerClassFactory(&Engine::instance());
        Mask::registerClassFactory(&Engine::instance());
        Normalize::registerClassFactory(&Engine::instance());
        Reflect::registerClassFactory(&Engine::instance());
        Refract::registerClassFactory(&Engine::instance());
        Split::registerClassFactory(&Engine::instance());
        Swizzle::registerClassFactory(&Engine::instance());

        CustomFunction::registerClassFactory(&Engine::instance());

        // Common
        NodeGroup::registerClassFactory(&Engine::instance());

        for(auto &it : Engine::factories()) {
            Url url(it.second);

            if(url.host() == "Shader") {
                TString path = url.path();
                if(path.front() == '/') {
                    path.removeFirst();
                }
                m_nodeTypes.push_back(path);
            }
        }
    }

    m_inputs.push_back({ "Diffuse",   Vector3(1.0, 1.0, 1.0), false });
    m_inputs.push_back({ "Emissive",  Vector3(0.0, 0.0, 0.0), false });
    m_inputs.push_back({ "Normal",    Vector3(0.5, 0.5, 1.0), false });
    m_inputs.push_back({ "Metallic",  0.0f, false });
    m_inputs.push_back({ "Roughness", 0.0f, false });
    m_inputs.push_back({ "Opacity",   1.0f, false });
    m_inputs.push_back({ "IOR",       1.0f, false });

    m_inputs.push_back({ "Position Offset", Vector3(0.0, 0.0, 0.0), true });

    m_previewSettings.setMaterialType(ShaderRootNode::Surface);
    m_previewSettings.setLightModel(ShaderRootNode::Unlit);
    m_previewSettings.setDoubleSided(true);
}

ShaderGraph::~ShaderGraph() {
    cleanup();
}

void ShaderGraph::scanForCustomFunctions() {
    QStringList filter({"*.mtlf"});

    QStringList paths = {
        ":/shaders/functions",
        ProjectSettings::instance()->contentPath().data()
    };

    for(auto &path : paths) {

        QDirIterator it(path, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(it.hasNext()) {
            QString filePath = it.next();

            QFile file(filePath);
            if(file.open(QFile::ReadOnly | QFile::Text)) {
                pugi::xml_document doc;
                QByteArray data = file.readAll();
                if(doc.load_string(data.data()).status == pugi::status_ok) {
                    pugi::xml_node function = doc.document_element();

                    const char *name = function.attribute("name").as_string();

                    m_nodeTypes.push_back(name);
                    m_exposedFunctions[Url(name).baseName()] = filePath.toStdString();
                }
            }
        }
    }
}

GraphNode *ShaderGraph::nodeCreate(const TString &type, int &index) {
    GraphNode *node = dynamic_cast<GraphNode *>(Engine::objectCreate(type));
    if(node) {
        node->setGraph(this);
        node->setTypeName(type);
        node->setName(type);

        ShaderNode *function = dynamic_cast<ShaderNode *>(node);
        if(function) {
            function->createParams();
        } else {
            NodeGroup *group = dynamic_cast<NodeGroup *>(node);
            if(group) {
                group->setName("Comment");
            }
        }

        if(index == -1 || index > m_nodes.size()) {
            index = m_nodes.size();
            m_nodes.push_back(node);
        } else {
            m_nodes.insert(std::next(m_nodes.begin(), index), node);
        }
        return node;
    } else { // Self exposed function
        if(!type.isEmpty()) {
            CustomFunction *function = Engine::objectCreate<CustomFunction>(type);
            function->exposeFunction(m_exposedFunctions[type]);
            function->setGraph(this);

            if(index == -1 || index > m_nodes.size()) {
                index = m_nodes.size();
                m_nodes.push_back(function);
            } else {
                m_nodes.insert(std::next(m_nodes.begin(), index), function);
            }
            return function;
        }
    }

    return nullptr;
}

GraphNode *ShaderGraph::fallbackRoot() {
    GraphNode *node = Engine::objectCreate<ShaderRootNode>("ShaderRootNode");
    node->setGraph(this);
    m_nodes.push_front(node);

    return node;
}

void ShaderGraph::onNodesLoaded() {
    m_rootNode = nullptr;

    for(auto it : m_nodes) {
        ShaderRootNode *root = dynamic_cast<ShaderRootNode *>(it);
        if(root) {
            m_rootNode = root;
            break;
        }
    }

    if(m_rootNode->ports().empty()) {
        int i = 0;
        for(auto &it : m_inputs) {
            NodePort port(m_rootNode, false, (uint32_t)it.m_value.type(), i, it.m_name,
                          ShaderNode::m_portColors[(uint32_t)it.m_value.type()], it.m_value);
            port.m_userFlags = it.m_vertex ? Vertex : Fragment;
            m_rootNode->ports().push_back(port);
            i++;
        }
    }
}

void ShaderGraph::nodeDelete(GraphNode *node) {
    AbstractNodeGraph::nodeDelete(node);

    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        delete it->second.instance;
        delete it->second.material;
        delete it->second.target;
        delete it->second.texture;
        m_previews.erase(it);
    }
}

StringList ShaderGraph::nodeList() const {
    return m_nodeTypes;
}

bool ShaderGraph::buildGraph(GraphNode *node) {
    if(node == nullptr) {
        node = m_rootNode;
    }
    cleanup();

    // Nodes
    setPragma("vertex", buildFrom(node, Vertex).toStdString());
    setPragma("fragment", buildFrom(node, Fragment).toStdString());

    TString layout;
    uint32_t binding = UNIFORM_BIND;

    // Textures
    uint16_t t = 0;
    for(auto &it : m_textures) {
        TString texture;
        if(it.second & ShaderRootNode::Cube) {
            texture += QString("layout(binding = %1) uniform samplerCube ").arg(binding).toStdString();
        } else {
            texture += QString("layout(binding = %1) uniform sampler2D ").arg(binding).toStdString();
        }
        texture += ((it.second & ShaderRootNode::Target) ? it.first : QString("texture%1").arg(t).toStdString()) + ";\n";
        layout.append(texture);

        t++;
        binding++;
    }

    layout.append("\n");

    setPragma("uniforms", layout);

    // Functions
    TString vertexFunctions;
    for(const auto &it : m_vertexFunctions) {
        vertexFunctions += it.second + '\n';
    }

    setPragma("vertexFunctions", vertexFunctions);

    TString fragmentFunctions;
    for(const auto &it : m_fragmentFunctions) {
        fragmentFunctions += it.second + '\n';
    }

    setPragma("fragmentFunctions", fragmentFunctions);

    return true;
}

VariantMap ShaderGraph::data(bool editor, ShaderRootNode *root) {
    if(root == nullptr) {
        root = m_rootNode;
    }

    Material::BlendState blendState;
    blendState.enabled = true;
    blendState.sourceColorBlendMode = Material::BlendFactor::One;
    blendState.sourceAlphaBlendMode = Material::BlendFactor::One;

    blendState.destinationColorBlendMode = Material::BlendFactor::One;
    blendState.destinationAlphaBlendMode = Material::BlendFactor::One;

    VariantMap user;
    VariantList properties;
    properties.push_back(root->materialType());
    properties.push_back(root->isDoubleSided());
    properties.push_back(root->lightModel());
    properties.push_back(root->isWireframe());

    user[PROPERTIES] = properties;
    user[BLENDSTATE] = ShaderBuilder::toVariant((root == m_rootNode) ? root->blendState() : blendState);
    user[DEPTHSTATE] = ShaderBuilder::toVariant((root == m_rootNode) ? root->depthState() : Material::DepthState());
    user[STENCILSTATE] = ShaderBuilder::toVariant((root == m_rootNode) ? root->stencilState() : Material::StencilState());

    VariantList textures;
    uint16_t i = 0;
    uint32_t binding = UNIFORM_BIND;
    for(auto &it : m_textures) {
        VariantList data;

        bool target = (it.second & ShaderRootNode::Target);

        data.push_back((target) ? "" : it.first); // path
        data.push_back((target) ? it.first : QString("texture%1").arg(i).toStdString()); // name
        data.push_back(binding); // binding
        data.push_back(it.second); // flags

        textures.push_back(data);
        ++i;
        ++binding;
    }

    VariantList uniforms;
    for(auto &it : m_uniforms) {
        VariantList data;

        uint32_t size = MetaType::size(it.value.type());

        data.push_back(it.value);
        data.push_back(uint32_t(size * it.count));
        data.push_back(it.name);

        uniforms.push_back(data);
    }
    user[UNIFORMS] = uniforms;

    ShaderBuilder::buildInstanceData(user, m_pragmas);

    std::string define;
    if(root == m_rootNode) {
        define += "\n#define USE_GBUFFER";
    }

    if(ShaderBuilder::currentRhi() == ShaderBuilderSettings::Rhi::Metal) {
        define += "\n#define ORIGIN_TOP";
    }

    NativeCodeBuilder *builder = ProjectSettings::instance()->currentBuilder();
    if(root->materialType() == ShaderRootNode::Surface && builder && !builder->isEmbedded()) {
        define += "\n#define USE_SSBO";
    }

    if((root == m_rootNode) && root->lightModel() == ShaderRootNode::Lit) {
        define += "\n#define USE_TBN";
    }

    // Pixel shader
    std::string file = "Shader.frag";
    {
        Variant data = ShaderBuilder::loadIncludes(file, define, m_pragmas);
        if(data.isValid()) {
            user[FRAGMENT] = data;
        }
    }
    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        user[VISIBILITY] = ShaderBuilder::loadIncludes(file, define + "\n#define VISIBILITY_BUFFER", m_pragmas);
    }

    // Vertex shader
    file = "Static.vert";
    if((root != m_rootNode) || root->materialType() == ShaderRootNode::PostProcess) {
        file = "Fullscreen.vert";
    }
    Variant data = ShaderBuilder::loadIncludes(file, define, m_pragmas);
    if(data.isValid()) {
        user[STATIC] = data;
    }

    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        if(root->useWithSkinned()) {
            Variant data = ShaderBuilder::loadIncludes("Skinned.vert", define, m_pragmas);
            if(data.isValid()) {
                user[SKINNED] = data;
            }
        }
        if(root->useWithParticles()){
            Variant data = ShaderBuilder::loadIncludes("Billboard.vert", define, m_pragmas);
            if(data.isValid()) {
                user[PARTICLE] = data;
            }
        }
    }
    user[TEXTURES] = textures;

    return user;
}

int ShaderGraph::addTexture(const TString &path, Vector4 &sub, int32_t flags) {
    sub = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index = -1;
    auto it = std::find(m_textures.begin(), m_textures.end(), std::make_pair(path, flags));
    if(it != m_textures.end()) {
        index = std::distance(m_textures.begin(), it);
    }

    if(index == -1) {
        index = m_textures.size();
        m_textures.push_back({ path, flags });
    }
    return index;
}

void ShaderGraph::addUniform(const TString &name, uint8_t type, const Variant &value) {
    for(auto &it : m_uniforms) {
        if(it.name == name) {
            it.type = type;
            it.value = value;
            return;
        }
    }
    m_uniforms.push_back({name, type, 1, value});
}

void ShaderGraph::addVertexFunction(const TString &name, TString &code) {
    auto it = m_vertexFunctions.find(name);
    if(it == m_vertexFunctions.end()) {
        m_vertexFunctions[name] = code;
    }
}

void ShaderGraph::addFragmentFunction(const TString &name, TString &code) {
    auto it = m_fragmentFunctions.find(name);
    if(it == m_fragmentFunctions.end()) {
        m_fragmentFunctions[name] = code;
    }
}

TString ShaderGraph::buildFrom(GraphNode *node, Stage stage) {
    for(auto &it : m_nodes) {
        ShaderNode *node = dynamic_cast<ShaderNode *>(it);
        if(node) {
            node->reset();
        }
    }

    TString result;
    if(node == nullptr) {
        return result;
    }
    int32_t depth = 0;

    ShaderNode *f = dynamic_cast<ShaderNode *>(node);
    if(f) {
        if(stage == Vertex) {
            return result;
        }

        std::stack<TString> stack;
        GraphLink link;
        link.sender = f;
        for(auto &port : f->ports()) {
            if(port.m_out) {
                link.oport = &port;
                break;
            }
        }

        TString type = "\tEmissive = %1;\n";

        int32_t size = 0;
        int32_t index = f->build(result, stack, link, depth, size);
        if(index >= 0) {
            if(stack.empty()) {
                result.append(type.arg(ShaderNode::convert(TString("local") + TString::number(index), size, MetaType::VECTOR3)));
            } else {
                result.append(type.arg(ShaderNode::convert(stack.top(), size, MetaType::VECTOR3)));
                stack.pop();
            }
        } else {
            result.append(type.arg("vec3(0.0)"));
        }
        result.append("\tOpacity = 1.0;\n");
    } else {
        for(NodePort &port : node->ports()) { // Iterate all ports for the node
            if(port.m_out == false && port.m_userFlags == stage) {
                TString name = port.m_name;
                name.remove(' ');

                TString value;

                bool isDefault = true;
                const GraphLink *link = findLink(node, &port);
                if(link) {
                    ShaderNode *node = dynamic_cast<ShaderNode *>(link->sender);
                    if(node) {
                        std::stack<TString> stack;
                        int32_t size = 0;
                        int32_t index = node->build(result, stack, *link, depth, size);
                        if(index >= 0) {
                            if(stack.empty()) {
                                value = ShaderNode::convert(TString("local") + TString::number(index), size, port.m_type);
                            } else {
                                value = ShaderNode::convert(stack.top(), size, port.m_type);
                                stack.pop();
                            }
                            isDefault = false;
                        }
                    }
                }

                if(isDefault) { // Default value
                    switch(port.m_type) {
                        case MetaType::FLOAT: {
                            value = TString::number(port.m_var.toFloat());
                        } break;
                        case MetaType::VECTOR2: {
                            Vector2 v(port.m_var.toVector2());
                            value = TString("vec2(%1, %2)").arg(TString::number(v.x),
                                                                TString::number(v.y));
                        } break;
                        case MetaType::VECTOR3: {
                            Vector3 v(port.m_var.toVector3());
                            value = TString("vec3(%1, %2, %3)").arg(TString::number(v.x),
                                                                    TString::number(v.y),
                                                                    TString::number(v.z));
                        } break;
                        case MetaType::VECTOR4: {
                            Vector4 v(port.m_var.toVector4());
                            value = TString("vec4(%1, %2, %3, %4)").arg(TString::number(v.x),
                                                                        TString::number(v.y),
                                                                        TString::number(v.z),
                                                                        TString::number(v.w));
                        } break;
                        default: break;
                    }
                }

                result.append(TString("\t%1 = %2;\n").arg(name, value));
            }
        }
    }
    return result.toStdString();
}

void ShaderGraph::cleanup() {
    m_textures.clear();
    m_uniforms.clear();
    m_vertexFunctions.clear();
    m_fragmentFunctions.clear();
    m_pragmas.clear();
}

void ShaderGraph::setPragma(const TString &key, const TString &value) {
    m_pragmas[key] = value;
}

void ShaderGraph::onNodeUpdated() {
    GraphNode *node = dynamic_cast<GraphNode *>(sender());
    if(node) {
        markDirty(node);
    }
    emitSignal(_SIGNAL(graphUpdated()));
}

void ShaderGraph::setPreviewVisible(GraphNode *node, bool visible) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        it->second.isVisible = visible;
    }
}

void ShaderGraph::updatePreviews(CommandBuffer &buffer) {
    for(auto &it : m_previews) {
        if(it.second.isVisible) {
            if(it.second.isDirty) {
                if(buildGraph(it.first)) {
                    VariantMap data = ShaderGraph::data(true, &m_previewSettings);
                    ShaderBuilder::compileData(data);

                    it.second.material->loadUserData(data);
                    if(it.second.instance) {
                        delete it.second.instance;
                    }
                    it.second.instance = it.second.material->createInstance(Material::Static);
                    it.second.isDirty = false;
                }
            }
            buffer.setRenderTarget(it.second.target);
            buffer.drawMesh(PipelineContext::defaultPlane(), 0, Material::Translucent, *it.second.instance);
        }
    }
}

GraphNode *ShaderGraph::defaultNode() const {
    return m_rootNode;
}

void ShaderGraph::markDirty(GraphNode *node) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        it->second.isDirty = true;
    }

    for(auto &it : m_links) {
        if(it->sender == node && it->sender != it->receiver) {
            markDirty(it->receiver);
        }
    }
}

Texture *ShaderGraph::preview(GraphNode *node) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        return it->second.texture;
    }

    if(dynamic_cast<NodeGroup *>(node) == nullptr && node != m_rootNode) {
        TString name(Uuid::createUuid().toString());

        PreviewData data;
        data.texture = Engine::objectCreate<Texture>(name + "_tex");
        data.texture->setFormat(Texture::RGBA8);
        data.texture->setFlags(Texture::Render);
        data.texture->resize(150, 150);

        data.target = Engine::objectCreate<RenderTarget>(name + "_rt");
        data.target->setColorAttachment(0, data.texture);
        data.target->setClearFlags(RenderTarget::ClearColor);

        data.material = Engine::objectCreate<Material>();

        m_previews[node] = data;
        return data.texture;
    }

    return nullptr;
}
