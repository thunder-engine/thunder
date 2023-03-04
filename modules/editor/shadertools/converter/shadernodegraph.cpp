#include "shadernodegraph.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QUuid>

#include <QStack>

#include <QMetaProperty>

#include <sstream>
#include <algorithm>

#include <commandbuffer.h>
#include <pipelinecontext.h>
#include <resources/rendertarget.h>

#include <editor/graph/nodegroup.h>

#include "functions/camera.h"
#include "functions/constvalue.h"
#include "functions/coordinates.h"
#include "functions/materialparam.h"
#include "functions/mathfunction.h"
#include "functions/mathoperator.h"
#include "functions/surface.h"
#include "functions/texturesample.h"
#include "functions/time.h"
#include "functions/trigonometry.h"
#include "functions/logicoperator.h"
#include "functions/vectoroperator.h"

#include "shaderbuilder.h"

map<uint32_t, Vector4> ShaderNode::m_portColors = {
    { QMetaType::Void, Vector4(0.6f, 0.6f, 0.6f, 1.0f) },
    { QMetaType::Int, Vector4(0.22f, 0.46, 0.11f, 1.0f) },
    { QMetaType::Float, Vector4(0.16f, 0.52f, 0.80f, 1.0f) },
    { QMetaType::QVector2D, Vector4(0.95f, 0.26f, 0.21f, 1.0f) },
    { QMetaType::QVector3D, Vector4(0.41f, 0.19f, 0.62f, 1.0f) },
    { QMetaType::QVector4D, Vector4(0.94f, 0.76f, 0.20f, 1.0f) },
};

ShaderNodeGraph::ShaderNodeGraph() {

    // Constants
    qRegisterMetaType<ConstPi*>("ConstPi");
    qRegisterMetaType<ConstGoldenRatio*>("ConstGoldenRatio");
    qRegisterMetaType<ConstFloat*>("ConstFloat");
    qRegisterMetaType<ConstVector2*>("ConstVector2");
    qRegisterMetaType<ConstVector3*>("ConstVector3");
    qRegisterMetaType<ConstVector4*>("ConstVector4");
    m_nodeTypes << "ConstFloat" << "ConstVector2" << "ConstVector3" << "ConstVector4"<< "ConstPi" << "ConstGoldenRatio" ;

    // Camera
    qRegisterMetaType<CameraPosition*>("CameraPosition");
    qRegisterMetaType<CameraDirection*>("CameraDirection");
    qRegisterMetaType<ScreenSize*>("ScreenSize");
    qRegisterMetaType<ScreenPosition*>("ScreenPosition");
    m_nodeTypes << "CameraPosition" << "CameraDirection" << "ScreenSize" << "ScreenPosition";

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
    qRegisterMetaType<TextureSample*>("TextureSample");
    qRegisterMetaType<RenderTargetSample*>("RenderTargetSample");
    qRegisterMetaType<TextureSampleCube*>("TextureSampleCube");
    m_nodeTypes << "TextureSample" << "RenderTargetSample" << "TextureSampleCube";

    qRegisterMetaType<Mod*>("Mod");
    m_nodeTypes << "Mod";

    // Logic Operators
    qRegisterMetaType<If*>("If");
    m_nodeTypes << "If";

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
    m_nodeTypes << "Abs" << "Add" << "Ceil" << "Clamp" << "DDX" << "DDY" << "Divide" << "Exp" << "Exp2" << "Floor";
    m_nodeTypes << "Fract" << "FWidth" << "Mix" << "Logarithm" << "Logarithm2" << "Max" << "Min" << "Multiply";
    m_nodeTypes << "Power" << "Round" << "Sign" << "Smoothstep" << "SquareRoot" << "Step" << "Subtraction" << "Truncate";

    // Surface
    qRegisterMetaType<Fresnel*>("Fresnel");
    qRegisterMetaType<WorldBitangent*>("WorldBitangent");
    qRegisterMetaType<WorldNormal*>("WorldNormal");
    qRegisterMetaType<WorldPosition*>("WorldPosition");
    qRegisterMetaType<WorldTangent*>("WorldTangent");
    m_nodeTypes << "Fresnel" << "WorldBitangent" << "WorldNormal" << "WorldPosition" << "WorldTangent";

    // Trigonometry operators
    qRegisterMetaType<ArcCosine*>("ArcCosine");
    qRegisterMetaType<ArcSine*>("ArcSine");
    qRegisterMetaType<ArcTangent*>("ArcTangent");
    qRegisterMetaType<Cosine*>("Cosine");
    qRegisterMetaType<CosineHyperbolic*>("CosineHyperbolic");
    qRegisterMetaType<Sine*>("Sine");
    qRegisterMetaType<SineHyperbolic*>("SineHyperbolic");
    qRegisterMetaType<Tangent*>("Tangent");
    qRegisterMetaType<TangentHyperbolic*>("TangentHyperbolic");
    m_nodeTypes << "ArcCosine" << "ArcSine" << "ArcTangent" << "Cosine" << "CosineHyperbolic" << "Sine";
    m_nodeTypes << "SineHyperbolic" << "Tangent" << "TangentHyperbolic";

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
    m_previewSettings.setBlend(ShaderRootNode::Translucent);
    m_previewSettings.setLightModel(ShaderRootNode::Unlit);
    m_previewSettings.setDoubleSided(true);
}

ShaderNodeGraph::~ShaderNodeGraph() {
    cleanup();
}

GraphNode *ShaderNodeGraph::nodeCreate(const QString &path, int &index) {
    const QByteArray className = qPrintable(path + "*");
    const int type = QMetaType::type(className);
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        GraphNode *node = dynamic_cast<GraphNode *>(meta->newInstance());
        if(node) {
            node->setGraph(this);
            node->setType(qPrintable(path));

            ShaderNode *function = dynamic_cast<ShaderNode *>(node);
            if(function) {
                connect(function, &ShaderNode::updated, this, &ShaderNodeGraph::onNodeUpdated);
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
    }

    return nullptr;
}

GraphNode *ShaderNodeGraph::createRoot() {
    ShaderRootNode *result = new ShaderRootNode;
    result->setGraph(this);
    connect(result, &ShaderRootNode::graphUpdated, this, &ShaderNodeGraph::graphUpdated);

    int i = 0;
    for(auto &it : m_inputs) {
        NodePort port(result, false, (uint32_t)it.m_value.type(), i, qPrintable(it.m_name),
                      ShaderNode::m_portColors[(uint32_t)it.m_value.type()], it.m_value);
        port.m_userFlags = it.m_vertex;
        result->ports().push_back(port);
        i++;
    }

    return result;
}

void ShaderNodeGraph::nodeDelete(GraphNode *node) {
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

QStringList ShaderNodeGraph::nodeList() const {
    QStringList result;
    for(auto &it : m_nodeTypes) {
        const int type = QMetaType::type( qPrintable(it) );
        const QMetaObject *meta = QMetaType::metaObjectForType(type);
        if(meta) {
            int index = meta->indexOfClassInfo("Group");
            if(index != -1) {
                result << QString(meta->classInfo(index).value()) + "/" + it;
            }
        }
    }

    return result;
}

void ShaderNodeGraph::load(const QString &path) {

    AbstractNodeGraph::load(path);

    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);
    root->setObjectName(QFileInfo(path).baseName());

    emit graphUpdated();
}

void ShaderNodeGraph::loadGraph(const QVariantMap &data) {
    AbstractNodeGraph::loadGraph(data);

    blockSignals(true);
    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

    root->setMaterialType(static_cast<ShaderRootNode::Type>(m_data[TYPE].toInt()));
    root->setBlend(static_cast<ShaderRootNode::Blend>(m_data[BLEND].toInt()));
    root->setLightModel(static_cast<ShaderRootNode::LightModel>(m_data[MODEL].toInt()));
    root->setDoubleSided(m_data[SIDE].toBool());
    root->setDepthTest(m_data.contains(DEPTH) ? m_data[DEPTH].toBool() : true);
    root->setDepthWrite(m_data.contains(DEPTHWRITE) ? m_data[DEPTHWRITE].toBool() : true);
    root->setWireframe(m_data.contains(WIREFRAME) ? m_data[WIREFRAME].toBool() : false);
    loadTextures(m_data[TEXTURES].toMap());
    loadUniforms(m_data[UNIFORMS].toList());
    blockSignals(false);

    emit graphUpdated();
}

void ShaderNodeGraph::save(const QString &path) {
    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

    m_data[TYPE] = root->materialType();
    m_data[BLEND] = root->blend();
    m_data[MODEL] = root->lightModel();
    m_data[SIDE] = root->isDoubleSided();
    m_data[DEPTH] = root->isDepthTest();
    m_data[DEPTHWRITE] = root->isDepthWrite();
    m_data[WIREFRAME] = root->isWireframe();
    m_data[TEXTURES] = saveTextures();
    m_data[UNIFORMS] = saveUniforms();

    AbstractNodeGraph::save(path);
}

void ShaderNodeGraph::loadUserValues(GraphNode *node, const QVariantMap &values) {
    node->blockSignals(true);
    for(QString key : values.keys()) {
        if(static_cast<QMetaType::Type>(values[key].type()) == QMetaType::QVariantList) {
            QVariantList array = values[key].toList();
            switch(array.first().toInt()) {
                case QVariant::Color: {
                    node->setProperty(qPrintable(key), QColor(array.at(1).toInt(), array.at(2).toInt(),
                                                              array.at(3).toInt(), array.at(4).toInt()));
                } break;
                default: {
                    if(array.first().toString() == "Template") {
                        node->setProperty(qPrintable(key),
                                          QVariant::fromValue(Template(array.at(1).toString(),
                                                                       array.at(2).toUInt())));
                    }
                } break;
            }
        } else {
            node->setProperty(qPrintable(key), values[key]);
        }
    }
    node->blockSignals(false);
}

void ShaderNodeGraph::saveUserValues(GraphNode *node, QVariantMap &values) {
    const QMetaObject *meta = node->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        if(property.isUser(node)) {
            QVariant value = property.read(node);
            switch(value.userType()) {
                case QMetaType::Bool: {
                    values[property.name()] = value.toBool();
                } break;
                case QMetaType::QString: {
                    values[property.name()] = value.toString();
                } break;
                case QMetaType::Float: {
                    values[property.name()] = value.toFloat();
                } break;
                case QMetaType::QColor: {
                    QVariantList v;
                    v.push_back(static_cast<int32_t>(QVariant::Color));
                    QColor col = value.value<QColor>();
                    v.push_back(col.red());
                    v.push_back(col.green());
                    v.push_back(col.blue());
                    v.push_back(col.alpha());
                    values[property.name()] = v;
                } break;
                default: {
                    if(value.canConvert<Template>()) {
                        Template tmp = value.value<Template>();
                        QVariantList v;
                        v.push_back(value.typeName());
                        v.push_back(tmp.path);
                        v.push_back(tmp.type);
                        values[property.name()] = v;
                    }
                } break;
            }
        }
    }
}

void ShaderNodeGraph::loadTextures(const QVariantMap &data) {
    m_textures.clear();
    for(auto it : data.keys()) {
        m_textures.push_back({it, data.value(it).toInt()});
    }
}

QVariantMap ShaderNodeGraph::saveTextures() const {
    QVariantMap result;
    for(auto &it : m_textures) {
        result[it.first] = it.second;
    }
    return result;
}

void ShaderNodeGraph::loadUniforms(const QVariantList &data) {
    m_uniforms.clear();
    for(auto &it : data) {
        QVariantList value = it.toList();
        auto field = value.begin();
        QString name = field->toString();
        ++field;
        uint32_t type = field->toInt();
        ++field;
        size_t count = field->toInt();
        ++field;
        switch(type) {
            case QMetaType::Int: {
                m_uniforms.push_back({name, type, count, 0});
            } break;
            case QMetaType::Float: {
                m_uniforms.push_back({name, type, count, 0.0f});
            } break;
            case QMetaType::QVector2D: {
                 m_uniforms.push_back({name, type, count, QVector2D()});
            } break;
            case QMetaType::QVector3D: {
                m_uniforms.push_back({name, type, count, QVector3D()});
            } break;
            case QMetaType::QVector4D: {
                m_uniforms.push_back({name, type, count, QColor()});
            } break;
            default: break;
        }
    }
}

QVariantList ShaderNodeGraph::saveUniforms() const {
    QVariantList result;

    for(auto &it : m_uniforms) {
        QVariantList value;
        value.push_back(it.name);
        switch(it.type) {
            case QMetaType::Int: {
                value.push_back((int)QMetaType::Int);
            } break;
            case QMetaType::Float: {
                value.push_back((int)QMetaType::Float);
            } break;
            case QMetaType::QVector2D: {
                value.push_back((int)QMetaType::QVector2D);
            } break;
            case QMetaType::QVector3D: {
                value.push_back((int)QMetaType::QVector3D);
            } break;
            case QMetaType::QVector4D: {
                value.push_back((int)QMetaType::QVector4D);
            } break;
            default: break;
        }
        value.push_back((uint32_t)it.count);
        result.push_back(value);
    }
    return result;
}

bool ShaderNodeGraph::buildGraph(GraphNode *node) {
    if(node == nullptr) {
        node = m_rootNode;
    }
    cleanup();

    // Nodes
    QString vertex = buildFrom(node, true);
    addPragma("vertex", vertex.toStdString());

    QString fragment = buildFrom(node, false);
    addPragma("fragment", fragment.toStdString());

    QString layout;
    uint32_t binding = UNIFORM_BIND;
    if(!m_uniforms.empty()) {
        layout += QString("layout(binding = %1) uniform Uniforms {\n").arg(binding);

        // Make uniforms
        for(const auto &it : m_uniforms) {
            switch(it.type) {
                case QMetaType::Float:     layout += "\tfloat " + it.name + ";\n"; break;
                case QMetaType::QVector2D: layout += "\tvec2 " + it.name + ";\n"; break;
                case QMetaType::QVector3D: layout += "\tvec3 " + it.name + ";\n"; break;
                case QMetaType::QVector4D: layout += "\tvec4 " + it.name + ";\n"; break;
                default: break;
            }
        }

        layout.append("} uni;\n\n");

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

    addPragma("uniforms", layout.toStdString());

    return true;
}

VariantMap ShaderNodeGraph::data(bool editor, ShaderRootNode *root) const {
    if(root == nullptr) {
        root = static_cast<ShaderRootNode *>(m_rootNode);
    }

    VariantMap user;
    VariantList properties;
    properties.push_back(root->materialType());
    properties.push_back(root->isDoubleSided());
    properties.push_back((root->materialType() == ShaderRootNode::Surface) ?
                             (Material::Static | Material::Skinned | Material::Billboard | Material::Oriented) :
                             Material::Static);
    properties.push_back(root->blend());
    properties.push_back(root->lightModel());
    properties.push_back(root->isDepthTest());
    properties.push_back(root->isDepthWrite());
    properties.push_back(root->isWireframe());

    user[PROPERTIES] = properties;

    VariantList textures;
    uint16_t i = 0;
    uint32_t binding = UNIFORM_BIND + 1;
    for(auto &it : m_textures) {
        VariantList data;

        bool target = (it.second & ShaderRootNode::Target);
        QString name = (target) ? it.first : QString("texture%1").arg(i);

        data.push_back(((target) ? "" : it.first.toStdString())); // path
        data.push_back(binding); // binding
        data.push_back(name.toStdString()); // name
        data.push_back(it.second); // flags

        textures.push_back(data);
        ++i;
        ++binding;
    }

    VariantList uniforms;
    for(auto &it : m_uniforms) {
        VariantList data;

        uint32_t size = 0;
        Variant value;
        switch(it.type) {
            case QMetaType::Int: {
                value = Variant(it.value.toInt());
                size = sizeof(int);
            } break;
            case QMetaType::Float: {
                value = Variant(it.value.toFloat());
                size = sizeof(float);
            } break;
            case QMetaType::QVector2D: {
                QVector2D v = it.value.value<QVector2D>();
                value = Variant(Vector2(v.x(), v.y()));
                size = sizeof(Vector2);
            } break;
            case QMetaType::QVector3D: {
                QVector3D v = it.value.value<QVector3D>();
                value = Variant(Vector3(v.x(), v.y(), v.z()));
                size = sizeof(Vector3);
            } break;
            case QMetaType::QVector4D: {
                QColor c = it.value.value<QColor>();
                value = Variant(Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
                size = sizeof(Vector4);
            } break;
            default: break;
        }
        data.push_back(value);
        data.push_back(uint32_t(size * it.count));
        data.push_back("uni." + it.name.toStdString());

        uniforms.push_back(data);
    }
    user[UNIFORMS] = uniforms;

    QString define;
    switch(root->blend()) {
        case ShaderRootNode::Additive: {
            define = "#define BLEND_ADDITIVE 1";
        } break;
        case ShaderRootNode::Translucent: {
            define = "#define BLEND_TRANSLUCENT 1";
        } break;
        default: {
            define = "#define BLEND_OPAQUE 1";
        } break;
    }
    switch(root->lightModel()) {
        case ShaderRootNode::Lit: {
            define += "\n#define MODEL_LIT 1";

            VariantList data;
            data.push_back(""); // path
            data.push_back(LOCAL_BIND + 1); // binding
            data.push_back("radianceMap"); // name
            data.push_back(ShaderRootNode::Target); // flags
            textures.push_back(data);

        } break;
        case ShaderRootNode::Subsurface: {
            define += "\n#define MODEL_SUBSURFACE 1";
        } break;
        default: {
            define += "\n#define MODEL_UNLIT 1";
        } break;
    }

    QString fragment = "Shader.frag";
    {
        Variant data = ShaderBuilder::loadIncludes(fragment, define, m_pragmas).toStdString();
        if(data.isValid()) {
            user[SHADER] = data;
        }
    }
    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        define += "\n#define SIMPLE 1";
        Variant data = ShaderBuilder::loadIncludes(fragment, define, m_pragmas).toStdString();
        if(data.isValid()) {
            user[SIMPLE] = data;
        }
    }

    QString vertex = "Shader.vert";
    {
        QString localDefine = define;
        if(root->materialType() == ShaderRootNode::PostProcess) {
            localDefine += "\n#define TYPE_FULLSCREEN";
        } else {
            localDefine += "\n#define TYPE_STATIC";
        }

        Variant data = ShaderBuilder::loadIncludes(vertex, localDefine, m_pragmas).toStdString();
        if(data.isValid()) {
            user[STATIC] = data;
        }
    }
    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        {
            QString localDefine = define + "\n#define INSTANCING";
            Variant data = ShaderBuilder::loadIncludes(vertex, localDefine, m_pragmas).toStdString();
            if(data.isValid()) {
                user[INSTANCED] = data;
            }
        }
        {
            QString localDefine = define + "\n#define TYPE_BILLBOARD";
            Variant data = ShaderBuilder::loadIncludes(vertex, localDefine, m_pragmas).toStdString();
            if(data.isValid()) {
                user[PARTICLE] = data;
            }
        }
        {
            QString localDefine = define + "\n#define TYPE_SKINNED";
            Variant data = ShaderBuilder::loadIncludes(vertex, localDefine, m_pragmas).toStdString();
            if(data.isValid()) {
                user[SKINNED] = data;
            }
        }
    }
    user[TEXTURES] = textures;

    return user;
}

int ShaderNodeGraph::setTexture(const QString &path, Vector4 &sub, int32_t flags) {
    sub = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index = m_textures.indexOf({ path, flags });
    if(index == -1) {
        index = m_textures.size();
        m_textures.push_back({ path, flags });
    }
    return index;
}

void ShaderNodeGraph::addUniform(const QString &name, uint8_t type, const QVariant &value) {
    for(auto &it : m_uniforms) {
        if(it.name == name) {
            it.type = type;
            it.value = value;
            return;
        }
    }
    m_uniforms.push_back({name, type, 1, value});
}

QString ShaderNodeGraph::buildFrom(GraphNode *node, bool vertex) {
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
        if(vertex) {
            result = "\tvec3 PositionOffset = vec3(0.0f);\n";
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
        int32_t size = 0;
        int32_t index = f->build(result, stack, link, depth, size);
        if(index >= 0) {
            QString type = "\tvec3 Emissive";
            if(stack.isEmpty()) {
                result.append(QString("%1 = %2;\n").arg(type, ShaderNode::convert("local" + QString::number(index), size, QMetaType::QVector3D)));
            } else {
                result.append(QString("%1 = %2;\n").arg(type, ShaderNode::convert(stack.pop(), size, QMetaType::QVector3D)));
            }
        } else {
            result.append("\tvec3 Emissive = vec3(0.0);\n");
        }
        result.append("\tfloat Opacity = 1.0;\n");
    } else {
        for(NodePort &port : node->ports()) { // Iterate all ports for the node
            if(port.m_out == false && port.m_userFlags == vertex) {
                QString name = port.m_name.c_str();
                name.replace(" ", "");

                QString type;
                switch(port.m_type) {
                    case QMetaType::Float:     type = QString("\tfloat " + name); break;
                    case QMetaType::QVector2D: type = QString("\tvec2 "  + name); break;
                    case QMetaType::QVector3D: type = QString("\tvec3 "  + name); break;
                    case QMetaType::QVector4D: type = QString("\tvec4 "  + name); break;
                    default: break;
                }

                bool isDefault = true;

                QString function;
                const Link *link = findLink(node, &port);
                if(link) {
                    ShaderNode *node = dynamic_cast<ShaderNode *>(link->sender);
                    if(node) {
                        QStack<QString> stack;
                        int32_t size = 0;
                        int32_t index = node->build(function, stack, *link, depth, size);

                        if(index >= 0) {
                            if(stack.isEmpty()) {
                                function.append(QString("%1 = %2;\n").arg(type, ShaderNode::convert("local" + QString::number(index), size, port.m_type)));
                            } else {
                                function.append(QString("%1 = %2;\n").arg(type, ShaderNode::convert(stack.pop(), size, port.m_type)));
                            }
                            isDefault = false;
                        }
                    }
                }

                if(isDefault) { // Default value
                    function.append(type);
                    switch(port.m_type) {
                        case QMetaType::Float: {
                            function.append(" = " + QString::number(port.m_var.toFloat()) + ";\n");
                        } break;
                        case QMetaType::QVector2D: {
                            QVector2D v = port.m_var.value<QVector2D>();
                            function.append(QString(" = vec2(%1, %2);\n").arg(v.x()).arg(v.y()));
                        } break;
                        case QMetaType::QVector3D: {
                            QVector3D v = port.m_var.value<QVector3D>();
                            function.append(QString(" = vec3(%1, %2, %3);\n").arg(v.x()).arg(v.y()).arg(v.z()));
                        } break;
                        case QMetaType::QVector4D: {
                            QVector4D v = port.m_var.value<QVector4D>();
                            function.append(QString(" = vec4(%1, %2, %3, %4);\n").arg(v.x()).arg(v.y()).arg(v.z()).arg(v.w()));
                        } break;
                        default: break;
                    }
                }



                result.append(function);
            }
        }
    }
    return result;
}

void ShaderNodeGraph::cleanup() {
    m_textures.clear();
    m_uniforms.clear();
    m_pragmas.clear();
}

void ShaderNodeGraph::addPragma(const string &key, const string &value) {
    m_pragmas[key] = m_pragmas[key].append(value).append("\n");
}

void ShaderNodeGraph::onNodeUpdated() {
    GraphNode *node = dynamic_cast<GraphNode *>(sender());
    if(node) {
        markDirty(node);
    }
    emit graphUpdated();
}

void ShaderNodeGraph::setPreviewVisible(GraphNode *node, bool visible) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        it->second.isVisible = visible;
    }
}

void ShaderNodeGraph::updatePreviews(CommandBuffer &buffer) {
    buffer.setScreenProjection();
    for(auto &it : m_previews) {
        if(it.second.isVisible) {
            if(it.second.isDirty) {
                if(buildGraph(it.first)) {
                    it.second.material->loadUserData(data(true, &m_previewSettings));
                    if(it.second.instance) {
                        delete it.second.instance;
                    }
                    it.second.instance = it.second.material->createInstance();
                    it.second.isDirty = false;
                }
            }
            buffer.setRenderTarget(it.second.target);
            buffer.clearRenderTarget(true, Vector4(0, 0, 0, 1));
            buffer.drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::TRANSLUCENT, it.second.instance);
        }
    }
    buffer.resetViewProjection();
}

void ShaderNodeGraph::markDirty(GraphNode *node) {
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

Texture *ShaderNodeGraph::preview(GraphNode *node) {
    auto it = m_previews.find(node);
    if(it != m_previews.end()) {
        return it->second.texture;
    }
    if(dynamic_cast<NodeGroup *>(node) == nullptr && node != m_rootNode) {
        QString name = QUuid::createUuid().toString();
        PreviewData data;
        data.texture = Engine::objectCreate<Texture>((name + "_tex").toStdString());
        data.texture->setFormat(Texture::RGBA8);
        data.texture->setWidth(128);
        data.texture->setHeight(128);

        data.target = Engine::objectCreate<RenderTarget>((name + "_rt").toStdString());
        data.target->setColorAttachment(0, data.texture);

        data.material = Engine::objectCreate<Material>();
        data.instance = nullptr;
        data.isDirty = true;
        data.isVisible = false;

        m_previews[node] = data;
        return data.texture;
    }
    return nullptr;
}
