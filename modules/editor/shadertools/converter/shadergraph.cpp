#include "shadergraph.h"

#include <QDirIterator>

#include <algorithm>

#include <commandbuffer.h>
#include <pipelinecontext.h>
#include <resources/rendertarget.h>

#include <editor/graph/nodegroup.h>
#include <editor/projectsettings.h>

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
        ShaderRootNode::registerClassFactory(Engine::resourceSystem());

        scanForCustomFunctions();

        // Base
        ShaderNode::registerClassFactory(Engine::resourceSystem());

        // Constants
        ConstColor::registerClassFactory(Engine::resourceSystem());
        ConstPi::registerClassFactory(Engine::resourceSystem());
        ConstEuler::registerClassFactory(Engine::resourceSystem());
        ConstGoldenRatio::registerClassFactory(Engine::resourceSystem());
        ConstFloat::registerClassFactory(Engine::resourceSystem());
        ConstInt::registerClassFactory(Engine::resourceSystem());
        ConstVector2::registerClassFactory(Engine::resourceSystem());
        ConstVector3::registerClassFactory(Engine::resourceSystem());
        ConstVector4::registerClassFactory(Engine::resourceSystem());
        ConstMatrix3::registerClassFactory(Engine::resourceSystem());
        ConstMatrix4::registerClassFactory(Engine::resourceSystem());

        // ImageEffects
        Desaturate::registerClassFactory(Engine::resourceSystem());

        // Camera
        CameraPosition::registerClassFactory(Engine::resourceSystem());
        CameraDirection::registerClassFactory(Engine::resourceSystem());
        ScreenSize::registerClassFactory(Engine::resourceSystem());
        ScreenPosition::registerClassFactory(Engine::resourceSystem());
        ProjectionMatrix::registerClassFactory(Engine::resourceSystem());
        ExtractPosition::registerClassFactory(Engine::resourceSystem());

        // Coordinates
        TexCoord::registerClassFactory(Engine::resourceSystem());
        ProjectionCoord::registerClassFactory(Engine::resourceSystem());
        CoordPanner::registerClassFactory(Engine::resourceSystem());

        // Parameters
        ParamFloat::registerClassFactory(Engine::resourceSystem());
        ParamVector::registerClassFactory(Engine::resourceSystem());

        // Texture
        TextureFunction::registerClassFactory(Engine::resourceSystem());

        TextureObject::registerClassFactory(Engine::resourceSystem());
        TextureSample::registerClassFactory(Engine::resourceSystem());
        RenderTargetSample::registerClassFactory(Engine::resourceSystem());
        TextureSampleCube::registerClassFactory(Engine::resourceSystem());

        // Logic Operators
        If::registerClassFactory(Engine::resourceSystem());
        Compare::registerClassFactory(Engine::resourceSystem());

        // Math Operations
        Abs::registerClassFactory(Engine::resourceSystem());
        Add::registerClassFactory(Engine::resourceSystem());
        Ceil::registerClassFactory(Engine::resourceSystem());
        Clamp::registerClassFactory(Engine::resourceSystem());
        DDX::registerClassFactory(Engine::resourceSystem());
        DDY::registerClassFactory(Engine::resourceSystem());
        Divide::registerClassFactory(Engine::resourceSystem());
        Exp::registerClassFactory(Engine::resourceSystem());
        Exp2::registerClassFactory(Engine::resourceSystem());
        Floor::registerClassFactory(Engine::resourceSystem());
        Fract::registerClassFactory(Engine::resourceSystem());
        FWidth::registerClassFactory(Engine::resourceSystem());
        Mix::registerClassFactory(Engine::resourceSystem());
        Logarithm::registerClassFactory(Engine::resourceSystem());
        Logarithm10::registerClassFactory(Engine::resourceSystem());
        Logarithm2::registerClassFactory(Engine::resourceSystem());
        Max::registerClassFactory(Engine::resourceSystem());
        Min::registerClassFactory(Engine::resourceSystem());
        Multiply::registerClassFactory(Engine::resourceSystem());
        Power::registerClassFactory(Engine::resourceSystem());
        Round::registerClassFactory(Engine::resourceSystem());
        Sign::registerClassFactory(Engine::resourceSystem());
        Smoothstep::registerClassFactory(Engine::resourceSystem());
        SquareRoot::registerClassFactory(Engine::resourceSystem());
        Step::registerClassFactory(Engine::resourceSystem());

        MathOperation::registerClassFactory(Engine::resourceSystem());

        Subtraction::registerClassFactory(Engine::resourceSystem());
        Truncate::registerClassFactory(Engine::resourceSystem());
        InverseLerp::registerClassFactory(Engine::resourceSystem());
        Fmod::registerClassFactory(Engine::resourceSystem());
        Negate::registerClassFactory(Engine::resourceSystem());
        Saturate::registerClassFactory(Engine::resourceSystem());
        Scale::registerClassFactory(Engine::resourceSystem());
        ScaleAndOffset::registerClassFactory(Engine::resourceSystem());
        OneMinus::registerClassFactory(Engine::resourceSystem());
        Remainder::registerClassFactory(Engine::resourceSystem());
        RSqrt::registerClassFactory(Engine::resourceSystem());
        TriangleWave::registerClassFactory(Engine::resourceSystem());
        SquareWave::registerClassFactory(Engine::resourceSystem());
        SawtoothWave::registerClassFactory(Engine::resourceSystem());

        // Matrix operations
        MatrixOperation::registerClassFactory(Engine::resourceSystem());

        Determinant::registerClassFactory(Engine::resourceSystem());
        Inverse::registerClassFactory(Engine::resourceSystem());
        Transpose::registerClassFactory(Engine::resourceSystem());
        MakeMatrix::registerClassFactory(Engine::resourceSystem());

        // Surface
        Fresnel::registerClassFactory(Engine::resourceSystem());
        SurfaceDepth::registerClassFactory(Engine::resourceSystem());
        WorldBitangent::registerClassFactory(Engine::resourceSystem());
        WorldNormal::registerClassFactory(Engine::resourceSystem());
        WorldPosition::registerClassFactory(Engine::resourceSystem());
        WorldTangent::registerClassFactory(Engine::resourceSystem());

        // Trigonometry operators
        ArcCosine::registerClassFactory(Engine::resourceSystem());
        ArcSine::registerClassFactory(Engine::resourceSystem());
        ArcTangent::registerClassFactory(Engine::resourceSystem());
        ArcTangent2::registerClassFactory(Engine::resourceSystem());
        Cosine::registerClassFactory(Engine::resourceSystem());
        CosineHyperbolic::registerClassFactory(Engine::resourceSystem());
        Degrees::registerClassFactory(Engine::resourceSystem());
        Radians::registerClassFactory(Engine::resourceSystem());
        Sine::registerClassFactory(Engine::resourceSystem());
        SineHyperbolic::registerClassFactory(Engine::resourceSystem());
        Tangent::registerClassFactory(Engine::resourceSystem());
        TangentHyperbolic::registerClassFactory(Engine::resourceSystem());

        // Time
        CosTime::registerClassFactory(Engine::resourceSystem());
        DeltaTime::registerClassFactory(Engine::resourceSystem());
        SinTime::registerClassFactory(Engine::resourceSystem());
        Time::registerClassFactory(Engine::resourceSystem());

        // Vector Operators
        Append::registerClassFactory(Engine::resourceSystem());
        CrossProduct::registerClassFactory(Engine::resourceSystem());
        Distance::registerClassFactory(Engine::resourceSystem());
        DotProduct::registerClassFactory(Engine::resourceSystem());
        Length::registerClassFactory(Engine::resourceSystem());
        Mask::registerClassFactory(Engine::resourceSystem());
        Normalize::registerClassFactory(Engine::resourceSystem());
        Reflect::registerClassFactory(Engine::resourceSystem());
        Refract::registerClassFactory(Engine::resourceSystem());
        Split::registerClassFactory(Engine::resourceSystem());
        Swizzle::registerClassFactory(Engine::resourceSystem());

        CustomFunction::registerClassFactory(Engine::resourceSystem());

        // Common
        NodeGroup::registerClassFactory(Engine::resourceSystem());

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

    int i = 0;
    for(auto &it : m_inputs) {
        NodePort port(m_rootNode, false, (uint32_t)it.m_value.type(), i, it.m_name,
                      ShaderNode::m_portColors[(uint32_t)it.m_value.type()], it.m_value);
        port.m_userFlags = it.m_vertex ? Vertex : Fragment;
        m_rootNode->ports().push_back(port);
        i++;
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

    if(root->materialType() == ShaderRootNode::Surface && ProjectSettings::instance()->currentPlatformName() == "desktop") {
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
        Link link;
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
                const Link *link = findLink(node, &port);
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
