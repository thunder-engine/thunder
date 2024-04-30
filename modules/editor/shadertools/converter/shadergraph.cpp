#include "shadergraph.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QUuid>

#include <QStack>

#include <QMetaProperty>
#include <QDirIterator>

#include <sstream>
#include <algorithm>

#include <commandbuffer.h>
#include <pipelinecontext.h>
#include <resources/rendertarget.h>

#include <editor/graph/nodegroup.h>
#include <editor/projectsettings.h>

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

#include "shaderbuilder.h"

namespace {
    const char *gOldBlend("Blend");
    const char *gOldDepth("Depth");
    const char *gOldDepthWrite("DepthWrite");
    const char *gOldModel("Model");
    const char *gOldType("Type");
    const char *gOldSide("Side");
    const char *gOldWireFrame("Wireframe");

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

map<uint32_t, Vector4> ShaderNode::m_portColors = {
    { QMetaType::Void, Vector4(0.6f, 0.6f, 0.6f, 1.0f) },
    { QMetaType::Int, Vector4(0.22f, 0.46, 0.11f, 1.0f) },
    { QMetaType::Float, Vector4(0.16f, 0.52f, 0.80f, 1.0f) },
    { QMetaType::QVector2D, Vector4(0.95f, 0.26f, 0.21f, 1.0f) },
    { QMetaType::QVector3D, Vector4(0.41f, 0.19f, 0.62f, 1.0f) },
    { QMetaType::QVector4D, Vector4(0.94f, 0.76f, 0.20f, 1.0f) },
    { QMetaType::QTransform, Vector4(0.5f, 0.93f, 0.44f, 1.0f) },
    { QMetaType::QMatrix4x4, Vector4(0.5f, 0.93f, 0.44f, 1.0f) },
    { QMetaType::QImage, Vector4(0.93f, 0.5f, 0.07f, 1.0f) },
};

ShaderGraph::ShaderGraph() {
    m_version = ShaderBuilder::version();

    scanForCustomFunctions();

    // Constants
    qRegisterMetaType<ConstColor*>("ConstColor");
    qRegisterMetaType<ConstPi*>("ConstPi");
    qRegisterMetaType<ConstEuler*>("ConstEuler");
    qRegisterMetaType<ConstGoldenRatio*>("ConstGoldenRatio");
    qRegisterMetaType<ConstFloat*>("ConstFloat");
    qRegisterMetaType<ConstInt*>("ConstInt");
    qRegisterMetaType<ConstVector2*>("ConstVector2");
    qRegisterMetaType<ConstVector3*>("ConstVector3");
    qRegisterMetaType<ConstVector4*>("ConstVector4");
    qRegisterMetaType<ConstMatrix3*>("ConstMatrix3");
    qRegisterMetaType<ConstMatrix4*>("ConstMatrix4");
    m_nodeTypes << "ConstColor" << "ConstEuler" << "ConstFloat" << "ConstInt" << "ConstVector2" << "ConstVector3" << "ConstVector4";
    m_nodeTypes << "ConstMatrix3" << "ConstMatrix4" << "ConstPi" << "ConstGoldenRatio";

    // ImageEffects
    qRegisterMetaType<Desaturate*>("Desaturate");
    m_nodeTypes << "Desaturate";

    // Camera
    qRegisterMetaType<CameraPosition*>("CameraPosition");
    qRegisterMetaType<CameraDirection*>("CameraDirection");
    qRegisterMetaType<ScreenSize*>("ScreenSize");
    qRegisterMetaType<ScreenPosition*>("ScreenPosition");
    qRegisterMetaType<ProjectionMatrix*>("ProjectionMatrix");
    qRegisterMetaType<ExtractPosition*>("ExtractPosition");
    m_nodeTypes << "CameraPosition" << "CameraDirection" << "ScreenSize" << "ScreenPosition" << "ProjectionMatrix" << "ExtractPosition";

    // Coordinates
    qRegisterMetaType<TexCoord*>("TexCoord");
    qRegisterMetaType<ProjectionCoord*>("ProjectionCoord");
    qRegisterMetaType<CoordPanner*>("CoordPanner");
    m_nodeTypes << "TexCoord" << "ProjectionCoord" << "CoordPanner";

    // Parameters
    qRegisterMetaType<ParamFloat*>("ParamFloat");
    qRegisterMetaType<ParamVector*>("ParamVector");
    m_nodeTypes << "ParamFloat" << "ParamVector";

    // Texture
    qRegisterMetaType<TextureObject*>("TextureObject");
    qRegisterMetaType<TextureSample*>("TextureSample");
    qRegisterMetaType<RenderTargetSample*>("RenderTargetSample");
    qRegisterMetaType<TextureSampleCube*>("TextureSampleCube");
    m_nodeTypes << "TextureObject" << "TextureSample" << "RenderTargetSample" << "TextureSampleCube";

    // Logic Operators
    qRegisterMetaType<If*>("If");
    qRegisterMetaType<Compare*>("Compare");
    m_nodeTypes << "If"/* << "Compare"*/;

    // Math Operations
    qRegisterMetaType<Abs*>("Abs");
    qRegisterMetaType<Add*>("Add");
    qRegisterMetaType<Ceil*>("Ceil");
    qRegisterMetaType<Clamp*>("Clamp");
    qRegisterMetaType<DDX*>("DDX");
    qRegisterMetaType<DDY*>("DDY");
    qRegisterMetaType<Divide*>("Divide");
    qRegisterMetaType<Exp*>("Exp");
    qRegisterMetaType<Exp2*>("Exp2");
    qRegisterMetaType<Floor*>("Floor");
    qRegisterMetaType<Fract*>("Fract");
    qRegisterMetaType<FWidth*>("FWidth");
    qRegisterMetaType<Mix*>("Mix");
    qRegisterMetaType<Logarithm*>("Logarithm");
    qRegisterMetaType<Logarithm10*>("Logarithm10");
    qRegisterMetaType<Logarithm2*>("Logarithm2");
    qRegisterMetaType<Max*>("Max");
    qRegisterMetaType<Min*>("Min");
    qRegisterMetaType<Multiply*>("Multiply");
    qRegisterMetaType<Power*>("Power");
    qRegisterMetaType<Round*>("Round");
    qRegisterMetaType<Sign*>("Sign");
    qRegisterMetaType<Smoothstep*>("Smoothstep");
    qRegisterMetaType<SquareRoot*>("SquareRoot");
    qRegisterMetaType<Step*>("Step");
    qRegisterMetaType<Subtraction*>("Subtraction");
    qRegisterMetaType<Truncate*>("Truncate");
    qRegisterMetaType<InverseLerp*>("InverseLerp");
    qRegisterMetaType<Fmod*>("Fmod");
    qRegisterMetaType<Negate*>("Negate");
    qRegisterMetaType<Saturate*>("Saturate");
    qRegisterMetaType<Scale*>("Scale");
    qRegisterMetaType<ScaleAndOffset*>("ScaleAndOffset");
    qRegisterMetaType<OneMinus*>("OneMinus");
    qRegisterMetaType<Remainder*>("Remainder");
    qRegisterMetaType<RSqrt*>("RSqrt");
    qRegisterMetaType<TriangleWave*>("TriangleWave");
    qRegisterMetaType<SquareWave*>("SquareWave");
    qRegisterMetaType<SawtoothWave*>("SawtoothWave");
    m_nodeTypes << "Abs" << "Add" << "Ceil" << "Clamp" << "DDX" << "DDY" << "Divide" << "Exp" << "Exp2" << "Floor";
    m_nodeTypes << "Fract" << "FWidth" << "Mix" << "Logarithm" << "Logarithm10" << "Logarithm2" << "Max" << "Min" << "Multiply";
    m_nodeTypes << "Power" << "Round" << "Sign" << "Smoothstep" << "SquareRoot" << "Step" << "Subtraction" << "Truncate";
    m_nodeTypes << "InverseLerp" << "Fmod" << "Negate" << "TriangleWave" << "SquareWave" << "SawtoothWave" << "Saturate" << "OneMinus";
    m_nodeTypes << "Remainder" << "RSqrt" << "Scale" << "ScaleAndOffset";

    // Matrix operations
    qRegisterMetaType<Determinant*>("Determinant");
    qRegisterMetaType<Inverse*>("Inverse");
    qRegisterMetaType<Transpose*>("Transpose");
    qRegisterMetaType<MakeMatrix*>("MakeMatrix");
    m_nodeTypes << "Determinant" << "Inverse" << "Transpose" << "MakeMatrix";

    // Surface
    qRegisterMetaType<Fresnel*>("Fresnel");
    qRegisterMetaType<SurfaceDepth*>("SurfaceDepth");
    qRegisterMetaType<WorldBitangent*>("WorldBitangent");
    qRegisterMetaType<WorldNormal*>("WorldNormal");
    qRegisterMetaType<WorldPosition*>("WorldPosition");
    qRegisterMetaType<WorldTangent*>("WorldTangent");
    m_nodeTypes << "Fresnel" << "SurfaceDepth" << "WorldBitangent" << "WorldNormal" << "WorldPosition" << "WorldTangent";

    // Trigonometry operators
    qRegisterMetaType<ArcCosine*>("ArcCosine");
    qRegisterMetaType<ArcSine*>("ArcSine");
    qRegisterMetaType<ArcTangent*>("ArcTangent");
    qRegisterMetaType<ArcTangent2*>("ArcTangent2");
    qRegisterMetaType<Cosine*>("Cosine");
    qRegisterMetaType<CosineHyperbolic*>("CosineHyperbolic");
    qRegisterMetaType<Degrees*>("Degrees");
    qRegisterMetaType<Radians*>("Radians");
    qRegisterMetaType<Sine*>("Sine");
    qRegisterMetaType<SineHyperbolic*>("SineHyperbolic");
    qRegisterMetaType<Tangent*>("Tangent");
    qRegisterMetaType<TangentHyperbolic*>("TangentHyperbolic");
    m_nodeTypes << "ArcCosine" << "ArcSine" << "ArcTangent" << "ArcTangent2" << "Cosine" << "CosineHyperbolic" << "Degrees" << "Radians";
    m_nodeTypes << "Sine" << "SineHyperbolic" << "Tangent" << "TangentHyperbolic";

    // Time
    qRegisterMetaType<CosTime*>("CosTime");
    qRegisterMetaType<DeltaTime*>("DeltaTime");
    qRegisterMetaType<SinTime*>("SinTime");
    qRegisterMetaType<Time*>("Time");
    m_nodeTypes << "CosTime" << "DeltaTime" << "SinTime" << "Time";

    // Vector Operators
    qRegisterMetaType<Append*>("Append");
    qRegisterMetaType<CrossProduct*>("CrossProduct");
    qRegisterMetaType<Distance*>("Distance");
    qRegisterMetaType<DotProduct*>("DotProduct");
    qRegisterMetaType<Length*>("Length");
    qRegisterMetaType<Mask*>("Mask");
    qRegisterMetaType<Normalize*>("Normalize");
    qRegisterMetaType<Reflect*>("Reflect");
    qRegisterMetaType<Refract*>("Refract");
    qRegisterMetaType<Split*>("Split");
    qRegisterMetaType<Swizzle*>("Swizzle");
    m_nodeTypes << "Append" << "CrossProduct" << "Distance" << "DotProduct" << "Length" << "Mask" << "Normalize";
    m_nodeTypes << "Reflect" << "Refract" << "Split" << "Swizzle";

    // Common
    qRegisterMetaType<NodeGroup*>("NodeGroup");
    m_nodeTypes << "NodeGroup";

    m_inputs.push_back({ "Diffuse",   QVector3D(1.0, 1.0, 1.0), false });
    m_inputs.push_back({ "Emissive",  QVector3D(0.0, 0.0, 0.0), false });
    m_inputs.push_back({ "Normal",    QVector3D(0.5, 0.5, 1.0), false });
    m_inputs.push_back({ "Metallic",  0.0f, false });
    m_inputs.push_back({ "Roughness", 0.0f, false });
    m_inputs.push_back({ "Opacity",   1.0f, false });
    m_inputs.push_back({ "IOR",       1.0f, false });

    m_inputs.push_back({ "Position Offset", QVector3D(0.0, 0.0, 0.0), true });

    m_previewSettings.setMaterialType(ShaderRootNode::Surface);
    m_previewSettings.setLightModel(ShaderRootNode::Unlit);
    m_previewSettings.setDoubleSided(true);
}

ShaderGraph::~ShaderGraph() {
    cleanup();
}

void ShaderGraph::scanForCustomFunctions() {
    QStringList filter({"*.mtlf"});

    QStringList files;

    QStringList paths = {
        ":/shaders/functions",
        ProjectSettings::instance()->contentPath()
    };

    for(auto &path : paths) {
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

                    QString name = function.attribute("name");

                    m_nodeTypes << name;
                    m_exposedFunctions[QFileInfo(name).baseName()] = path;
                }
            }
        }
    }
}

GraphNode *ShaderGraph::nodeCreate(const QString &path, int &index) {
    const QByteArray className = qPrintable(path + "*");
    const int type = QMetaType::type(className);
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        GraphNode *node = dynamic_cast<GraphNode *>(meta->newInstance());
        if(node) {
            node->setGraph(this);
            node->setTypeName(qPrintable(path));

            ShaderNode *function = dynamic_cast<ShaderNode *>(node);
            if(function) {
                function->createParams();
                connect(function, &ShaderNode::updated, this, &ShaderGraph::onNodeUpdated);
            } else {
                NodeGroup *group = dynamic_cast<NodeGroup *>(node);
                if(group) {
                    group->setObjectName("Comment");
                }
            }

            if(index == -1) {
                index = m_nodes.size();
                m_nodes.push_back(node);
            } else {
                m_nodes.insert(index, node);
            }
            return node;
        }
    } else { // Self exposed function
        if(!path.isEmpty()) {
            CustomFunction *function = new CustomFunction();
            function->exposeFunction(m_exposedFunctions[path]);
            function->setGraph(this);
            connect(function, &ShaderNode::updated, this, &ShaderGraph::onNodeUpdated);

            if(index == -1) {
                index = m_nodes.size();
                m_nodes.push_back(function);
            } else {
                m_nodes.insert(index, function);
            }
            return function;
        }
    }

    return nullptr;
}

GraphNode *ShaderGraph::createRoot() {
    ShaderRootNode *result = new ShaderRootNode;
    result->setGraph(this);
    connect(result, &ShaderRootNode::graphUpdated, this, &ShaderGraph::graphUpdated);

    int i = 0;
    for(auto &it : m_inputs) {
        NodePort port(result, false, (uint32_t)it.m_value.type(), i, it.m_name.toStdString(),
                      ShaderNode::m_portColors[(uint32_t)it.m_value.type()], it.m_value);
        port.m_userFlags = it.m_vertex ? Vertex : Fragment;
        result->ports().push_back(port);
        i++;
    }

    return result;
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

QStringList ShaderGraph::nodeList() const {
    QStringList result;
    for(auto &it : m_nodeTypes) {
        const int type = QMetaType::type( qPrintable(it) );
        const QMetaObject *meta = QMetaType::metaObjectForType(type);
        if(meta) {
            int index = meta->indexOfClassInfo("Group");
            if(index != -1) {
                result << QString(meta->classInfo(index).value()) + "/" + it;
            }
        } else { // Self exposed function
            result << it;
        }
    }

    result.sort();

    return result;
}

void ShaderGraph::loadGraphV0(const QVariantMap &data) {
    AbstractNodeGraph::loadGraphV0(data);

    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);
    root->blockSignals(true);

    root->setMaterialType(static_cast<ShaderRootNode::Type>(data[gOldType].toInt()));
    root->setLightModel(static_cast<ShaderRootNode::LightModel>(data[gOldModel].toInt()));
    root->setDoubleSided(data[gOldSide].toBool());
    root->setDepthTest(data.contains(gOldDepth) ? data[gOldDepth].toBool() : true);
    root->setDepthWrite(data.contains(gOldDepthWrite) ? data[gOldDepthWrite].toBool() : true);
    root->setWireframe(data.contains(gOldWireFrame) ? data[gOldWireFrame].toBool() : false);

    root->setBlendState(ShaderBuilder::fromBlendMode(data[gOldBlend].toInt()));

    root->blockSignals(false);

    emit graphUpdated();
}

void ShaderGraph::loadGraphV11(const QDomElement &parent) {
    AbstractNodeGraph::loadGraphV11(parent);

    if(parent.tagName() == gUser) {
        ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

        const QMetaObject *meta = root->metaObject();

        QDomElement type = parent.firstChildElement(gType);
        if(!type.isNull()) {
            int32_t index = meta->indexOfEnumerator("Type");
            if(index > -1) {
                QMetaEnum metaEnum = meta->enumerator(index);
                root->setMaterialType( static_cast<ShaderRootNode::Type>(metaEnum.keyToValue(qPrintable(type.attribute(gValue)))) );
            }
        }

        QDomElement model = parent.firstChildElement(gModel);
        if(!model.isNull()) {
            int32_t index = meta->indexOfEnumerator("LightModel");
            if(index > -1) {
                QMetaEnum metaEnum = meta->enumerator(index);
                root->setLightModel( static_cast<ShaderRootNode::LightModel>(metaEnum.keyToValue(qPrintable(model.attribute(gValue)))));
            }
        }

        QDomElement side = parent.firstChildElement(gSide);
        if(!side.isNull()) {
            root->setDoubleSided(side.attribute(gValue) == "true");
        }

        QDomElement wire = parent.firstChildElement(gWireFrame);
        if(!wire.isNull()) {
            root->setWireframe(wire.attribute(gValue) == "true");
        }

        root->setBlendState(ShaderBuilder::loadBlendState(parent.firstChildElement("blend")));
        root->setDepthState(ShaderBuilder::loadDepthState(parent.firstChildElement("depth")));
        root->setStencilState(ShaderBuilder::loadStencilState(parent.firstChildElement("stencil")));
    }
}

void ShaderGraph::saveGraph(QDomElement parent, QDomDocument xml) const {
    AbstractNodeGraph::saveGraph(parent, xml);

    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

    QDomElement user = xml.createElement(gUser);

    const QMetaObject *meta = root->metaObject();

    int32_t index = meta->indexOfEnumerator("Type");
    if(index > -1) {
        QDomElement type = xml.createElement(gType);
        QMetaEnum metaEnum = meta->enumerator(index);
        type.setAttribute(gValue, metaEnum.key(root->materialType()));
        user.appendChild(type);
    }

    index = meta->indexOfEnumerator("LightModel");
    if(index > -1) {
        QMetaEnum metaEnum = meta->enumerator(index);
        QDomElement modelElement = xml.createElement(gModel);
        ShaderRootNode::LightModel model = root->lightModel();
        const char *key = metaEnum.key(model);
        modelElement.setAttribute(gValue, key);
        user.appendChild(modelElement);
    }

    QDomElement side = xml.createElement(gSide);
    side.setAttribute(gValue, root->isDoubleSided() ? "true" : "false");
    user.appendChild(side);

    QDomElement wire = xml.createElement(gWireFrame);
    wire.setAttribute(gValue, root->isWireframe() ? "true" : "false");
    user.appendChild(wire);

    ShaderBuilder::saveBlendState(root->blendState(), xml, user);

    if(root->depthTest()) {
        ShaderBuilder::saveDepthState(root->depthState(), xml, user);
    }

    if(root->stencilTest()) {
        ShaderBuilder::saveStencilState(root->stencilState(), xml, user);
    }

    parent.appendChild(user);
}

void ShaderGraph::loadUserValues(GraphNode *node, const QVariantMap &values) {
    node->blockSignals(true);
    node->loadUserData(values);
    node->blockSignals(false);
}

void ShaderGraph::saveUserValues(GraphNode *node, QVariantMap &values) const {
    node->saveUserData(values);
}

bool ShaderGraph::buildGraph(GraphNode *node) {
    if(node == nullptr) {
        node = m_rootNode;
    }
    cleanup();

    // Nodes
    setPragma("vertex", buildFrom(node, Vertex).toStdString());
    setPragma("fragment", buildFrom(node, Fragment).toStdString());

    // Uniforms
    QString layout;
    uint32_t binding = UNIFORM_BIND;
    if(!m_uniforms.empty()) {
        layout += "layout(binding = UNIFORM) uniform Uniforms {\n";

        // Make uniforms
        for(const auto &it : m_uniforms) {
            layout += QString("\t%1 %2;\n").arg(ShaderNode::typeToString(it.type), it.name);
        }

        layout.append("} uni;\n");

        binding++;
    }

    // Textures
    uint16_t t = 0;
    for(auto &it : m_textures) {
        QString texture;
        if(it.second & ShaderRootNode::Cube) {
            texture += QString("layout(binding = %1) uniform samplerCube ").arg(binding);
        } else {
            texture += QString("layout(binding = %1) uniform sampler2D ").arg(binding);
        }
        texture += ((it.second & ShaderRootNode::Target) ? it.first : QString("texture%1").arg(t)) + ";\n";
        layout.append(texture);

        t++;
        binding++;
    }

    layout.append("\n");

    setPragma("uniforms", layout.toStdString());

    // Functions
    QString functions;
    for(const auto &it : m_functions) {
        functions += it.second + "\n";
    }

    setPragma("functions", functions.toStdString());

    return true;
}

VariantMap ShaderGraph::data(bool editor, ShaderRootNode *root) {
    if(root == nullptr) {
        root = static_cast<ShaderRootNode *>(m_rootNode);
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
    uint32_t binding = UNIFORM_BIND + 1;
    for(auto &it : m_textures) {
        VariantList data;

        bool target = (it.second & ShaderRootNode::Target);

        data.push_back((target) ? "" : it.first.toStdString()); // path
        data.push_back(binding); // binding
        data.push_back((target) ? it.first.toStdString() : QString("texture%1").arg(i).toStdString()); // name
        data.push_back(it.second); // flags

        textures.push_back(data);
        ++i;
        ++binding;
    }

    VariantList uniforms;
    for(auto &it : m_uniforms) {
        VariantList data;

        Variant value = ShaderNode::fromQVariant(it.value);
        uint32_t size = MetaType::size(value.type());

        data.push_back(value);
        data.push_back(uint32_t(size * it.count));
        data.push_back(it.name.toStdString());

        uniforms.push_back(data);
    }
    user[UNIFORMS] = uniforms;

    ShaderBuilder::buildInstanceData(user, m_pragmas);

    Material::BlendState blend = root->blendState();

    string define;
    if(root == m_rootNode) {
        define += "\n#define USE_GBUFFER";
    }

    if(root->materialType() == ShaderRootNode::Surface && ProjectSettings::instance()->currentPlatformName() == "desktop") {
        define += "\n#define USE_SSBO";
    }

    if((root == m_rootNode) && root->lightModel() == ShaderRootNode::Lit) {
        define += "\n#define USE_TBN";

        VariantList data;
        data.push_back(""); // path
        data.push_back(LOCAL_BIND + 1); // binding
        data.push_back("radianceMap"); // name
        data.push_back(ShaderRootNode::Target); // flags
        textures.push_back(data);
    }

    // Pixel shader
    string file = "Shader.frag";
    {
        Variant data = ShaderBuilder::loadIncludes(file, define, m_pragmas);
        if(data.isValid()) {
            user[FRAGMENT] = data;
        }
    }
    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        Variant data = ShaderBuilder::loadIncludes(file, define + "\n#define VISIBILITY_BUFFER", m_pragmas);
        if(data.isValid()) {
            user[VISIBILITY] = data;
        }
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

                VariantList data;
                data.push_back(""); // path
                data.push_back(LOCAL_BIND + 2); // binding
                data.push_back("skinMatrices"); // name
                data.push_back(ShaderRootNode::Target); // flags
                textures.push_back(data);
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

int ShaderGraph::addTexture(const QString &path, Vector4 &sub, int32_t flags) {
    sub = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index = m_textures.indexOf({ path, flags });
    if(index == -1) {
        index = m_textures.size();
        m_textures.push_back({ path, flags });
    }
    return index;
}

void ShaderGraph::addUniform(const QString &name, uint8_t type, const QVariant &value) {
    for(auto &it : m_uniforms) {
        if(it.name == name) {
            it.type = type;
            it.value = value;
            return;
        }
    }
    m_uniforms.push_back({name, type, 1, value});
}

void ShaderGraph::addFunction(const QString &name, QString &code) {
    auto it = m_functions.find(name);
    if(it == m_functions.end()) {
        m_functions[name] = code;
    }
}

QString ShaderGraph::buildFrom(GraphNode *node, Stage stage) {
    for(auto &it : m_nodes) {
        ShaderNode *node = dynamic_cast<ShaderNode *>(it);
        if(node) {
            node->reset();
        }
    }

    QString result;
    if(node == nullptr) {
        return result;
    }
    int32_t depth = 0;

    ShaderNode *f = dynamic_cast<ShaderNode *>(node);
    if(f) {
        if(stage == Vertex) {
            return result;
        }

        QStack<QString> stack;
        Link link;
        link.sender = f;
        for(auto &port : f->ports()) {
            if(port.m_out) {
                link.oport = &port;
                break;
            }
        }

        QString type = "\tEmissive = %1;\n";

        int32_t size = 0;
        int32_t index = f->build(result, stack, link, depth, size);
        if(index >= 0) {
            if(stack.isEmpty()) {
                result.append(type.arg(ShaderNode::convert("local" + QString::number(index), size, QMetaType::QVector3D)));
            } else {
                result.append(type.arg(ShaderNode::convert(stack.pop(), size, QMetaType::QVector3D)));
            }
        } else {
            result.append(type.arg("vec3(0.0)"));
        }
        result.append("\tOpacity = 1.0;\n");
    } else {
        for(NodePort &port : node->ports()) { // Iterate all ports for the node
            if(port.m_out == false && port.m_userFlags == stage) {
                QString name = port.m_name.c_str();
                name.replace(" ", "");

                QString value;

                bool isDefault = true;
                const Link *link = findLink(node, &port);
                if(link) {
                    ShaderNode *node = dynamic_cast<ShaderNode *>(link->sender);
                    if(node) {
                        QStack<QString> stack;
                        int32_t size = 0;
                        int32_t index = node->build(result, stack, *link, depth, size);
                        if(index >= 0) {
                            if(stack.isEmpty()) {
                                value = ShaderNode::convert("local" + QString::number(index), size, port.m_type);
                            } else {
                                value = ShaderNode::convert(stack.pop(), size, port.m_type);
                            }
                            isDefault = false;
                        }
                    }
                }

                if(isDefault) { // Default value
                    switch(port.m_type) {
                        case QMetaType::Float: {
                            value = QString::number(port.m_var.toFloat());
                        } break;
                        case QMetaType::QVector2D: {
                            QVector2D v = port.m_var.value<QVector2D>();
                            value = QString("vec2(%1, %2)").arg(QString::number(v.x()),
                                                                QString::number(v.y()));
                        } break;
                        case QMetaType::QVector3D: {
                            QVector3D v = port.m_var.value<QVector3D>();
                            value = QString("vec3(%1, %2, %3)").arg(QString::number(v.x()),
                                                                    QString::number(v.y()),
                                                                    QString::number(v.z()));
                        } break;
                        case QMetaType::QVector4D: {
                            QVector4D v = port.m_var.value<QVector4D>();
                            value = QString("vec4(%1, %2, %3, %4)").arg(QString::number(v.x()),
                                                                        QString::number(v.y()),
                                                                        QString::number(v.z()),
                                                                        QString::number(v.w()));
                        } break;
                        default: break;
                    }
                }

                result.append(QString("\t%1 = %2;\n").arg(name, value));
            }
        }
    }
    return result;
}

void ShaderGraph::cleanup() {
    m_textures.clear();
    m_uniforms.clear();
    m_functions.clear();
    m_pragmas.clear();
}

void ShaderGraph::setPragma(const string &key, const string &value) {
    m_pragmas[key] = value;
}

void ShaderGraph::onNodeUpdated() {
    GraphNode *node = dynamic_cast<GraphNode *>(sender());
    if(node) {
        markDirty(node);
    }
    emit graphUpdated();
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
            buffer.clearRenderTarget(true, Vector4(0, 0, 0, 1));
            buffer.drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::TRANSLUCENT, *it.second.instance);
        }
    }
}

void ShaderGraph::markDirty(GraphNode *node) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        it->second.isDirty = true;
    }

    for(auto &it : m_links) {
        if(it->sender == node) {
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
        QString name = QUuid::createUuid().toString();

        PreviewData data;
        data.texture = Engine::objectCreate<Texture>((name + "_tex").toStdString());
        data.texture->setFormat(Texture::RGBA8);
        data.texture->setFlags(Texture::Render);
        data.texture->resize(150, 150);

        data.target = Engine::objectCreate<RenderTarget>((name + "_rt").toStdString());
        data.target->setColorAttachment(0, data.texture);

        data.material = Engine::objectCreate<Material>();

        m_previews[node] = data;
        return data.texture;
    }

    return nullptr;
}
