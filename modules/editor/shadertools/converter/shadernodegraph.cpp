#include "shadernodegraph.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

#include <QStack>

#include <QMetaProperty>

#include <sstream>
#include <algorithm>

#include <commandbuffer.h>

#include <editor/graph/nodegroup.h>

#include "functions/constvalue.h"
#include "functions/coordinates.h"
#include "functions/materialparam.h"
#include "functions/mathfunction.h"
#include "functions/mathoperator.h"
#include "functions/texturesample.h"
#include "functions/utils.h"

#include "shaderbuilder.h"

map<uint32_t, Vector4> ShaderFunction::m_portColors = {
    { QMetaType::Void, Vector4(0.6f, 0.6f, 0.6f, 1.0f) },
    { QMetaType::Int, Vector4(0.22f, 0.46, 0.11f, 1.0f) },
    { QMetaType::Float, Vector4(0.16f, 0.52f, 0.80f, 1.0f) },
    { QMetaType::QVector2D, Vector4(0.95f, 0.26f, 0.21f, 1.0f) },
    { QMetaType::QVector3D, Vector4(0.41f, 0.19f, 0.62f, 1.0f) },
    { QMetaType::QVector4D, Vector4(0.94f, 0.76f, 0.20f, 1.0f) },
};

ShaderNodeGraph::ShaderNodeGraph() {

    qRegisterMetaType<ConstFloat*>("ConstFloat");
    qRegisterMetaType<ConstVector2*>("ConstVector2");
    qRegisterMetaType<ConstVector3*>("ConstVector3");
    qRegisterMetaType<ConstVector4*>("ConstVector4");
    m_functions << "ConstFloat" << "ConstVector2" << "ConstVector3" << "ConstVector4";

    qRegisterMetaType<TexCoord*>("TexCoord");
    qRegisterMetaType<NormalVectorWS*>("NormalVectorWS");
    qRegisterMetaType<CameraPosition*>("CameraPosition");
    qRegisterMetaType<CameraDirection*>("CameraDirection");
    qRegisterMetaType<ProjectionCoord*>("ProjectionCoord");
    qRegisterMetaType<CoordPanner*>("CoordPanner");
    m_functions << "TexCoord" << "NormalVectorWS" << "CameraPosition" << "CameraDirection" << "ProjectionCoord" << "CoordPanner";

    qRegisterMetaType<ParamFloat*>("ParamFloat");
    qRegisterMetaType<ParamVector*>("ParamVector");
    m_functions << "ParamFloat" << "ParamVector";

    qRegisterMetaType<TextureSample*>("TextureSample");
    qRegisterMetaType<RenderTargetSample*>("RenderTargetSample");
    qRegisterMetaType<TextureSampleCube*>("TextureSampleCube");
    m_functions << "TextureSample" << "RenderTargetSample" << "TextureSampleCube";

    qRegisterMetaType<DotProduct*>("DotProduct");
    qRegisterMetaType<CrossProduct*>("CrossProduct");
    qRegisterMetaType<Smoothstep*>("Smoothstep");
    qRegisterMetaType<Mix*>("Mix");
    qRegisterMetaType<Mod*>("Mod");
    qRegisterMetaType<Power*>("Power");
    qRegisterMetaType<SquareRoot*>("SquareRoot");
    qRegisterMetaType<Logarithm*>("Logarithm");
    qRegisterMetaType<Logarithm2*>("Logarithm2");
    qRegisterMetaType<FWidth*>("FWidth");
    m_functions << "DotProduct" << "CrossProduct" << "Mix" << "Smoothstep" << "Mod" << "Power" << "SquareRoot" << "Logarithm" << "Logarithm2" << "FWidth";

    qRegisterMetaType<Clamp*>("Clamp");
    qRegisterMetaType<Min*>("Min");
    qRegisterMetaType<Max*>("Max");
    qRegisterMetaType<Abs*>("Abs");
    qRegisterMetaType<Sign*>("Sign");
    qRegisterMetaType<Floor*>("Floor");
    qRegisterMetaType<Ceil*>("Ceil");
    qRegisterMetaType<Round*>("Round");
    qRegisterMetaType<Truncate*>("Truncate");
    qRegisterMetaType<Fract*>("Fract");
    qRegisterMetaType<Normalize*>("Normalize");
    m_functions << "Clamp" << "Min" << "Max" << "Abs" << "Sign" << "Floor" << "Ceil" << "Round" << "Truncate" << "Fract" << "Normalize";

    qRegisterMetaType<Sine*>("Sine");
    qRegisterMetaType<Cosine*>("Cosine");
    qRegisterMetaType<Tangent*>("Tangent");
    qRegisterMetaType<ArcSine*>("ArcSine");
    qRegisterMetaType<ArcCosine*>("ArcCosine");
    qRegisterMetaType<ArcTangent*>("ArcTangent");
    m_functions << "Sine" << "Cosine" << "Tangent" << "ArcSine" << "ArcCosine" << "ArcTangent";

    qRegisterMetaType<Subtraction*>("Subtraction");
    qRegisterMetaType<Add*>("Add");
    qRegisterMetaType<Divide*>("Divide");
    qRegisterMetaType<Multiply*>("Multiply");
    m_functions << "Subtraction" << "Add" << "Divide" << "Multiply";

    qRegisterMetaType<Mask*>("Mask");
    qRegisterMetaType<Fresnel*>("Fresnel");
    qRegisterMetaType<If*>("If");
    m_functions << "Mask" << "Fresnel" << "If";

    qRegisterMetaType<NodeGroup*>("NodeGroup");
    m_functions << "NodeGroup";

    m_inputs.push_back({ "Diffuse",   QVector3D(1.0, 1.0, 1.0), false });
    m_inputs.push_back({ "Emissive",  QVector3D(0.0, 0.0, 0.0), false });
    m_inputs.push_back({ "Normal",    QVector3D(0.5, 0.5, 1.0), false });
    m_inputs.push_back({ "Metallic",  0.0f, false });
    m_inputs.push_back({ "Roughness", 0.0f, false });
    m_inputs.push_back({ "Opacity",   1.0f, false });
    m_inputs.push_back({ "IOR",       1.0f, false });

    m_inputs.push_back({ "Position Offset", QVector3D(0.0, 0.0, 0.0), true });
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

            ShaderFunction *function = dynamic_cast<ShaderFunction *>(node);
            if(function) {
                connect(function, &ShaderFunction::updated, this, &ShaderNodeGraph::graphUpdated);
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
        result->ports().push_back( NodePort(result, false, (uint32_t)it.m_value.type(), i, qPrintable(it.m_name),
                                            ShaderFunction::m_portColors[(uint32_t)it.m_value.type()], it.m_value) );
        i++;
    }

    return result;
}

QStringList ShaderNodeGraph::nodeList() const {
    QStringList result;
    for(auto &it : m_functions) {
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
    blockSignals(true);
    AbstractNodeGraph::load(path);

    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

    root->setObjectName(QFileInfo(path).baseName());

    root->setMaterialType(static_cast<ShaderRootNode::Type>(m_data[TYPE].toInt()));
    root->setBlend(static_cast<ShaderRootNode::Blend>(m_data[BLEND].toInt()));
    root->setLightModel(static_cast<ShaderRootNode::LightModel>(m_data[MODEL].toInt()));
    root->setDoubleSided(m_data[SIDE].toBool());
    root->setDepthTest(m_data.contains(DEPTH) ? m_data[DEPTH].toBool() : true);
    root->setDepthWrite(m_data.contains(DEPTHWRITE) ? m_data[DEPTHWRITE].toBool() : true);
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

bool ShaderNodeGraph::buildGraph() {
    cleanup();

    // Nodes
    QStringList functions = buildFrom(m_rootNode);

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
    uint16_t i = 0;
    for(auto &it : m_textures) {
        QString texture;
        if(it.second & ShaderRootNode::Cube) {
            texture += QString("layout(binding = %1) uniform samplerCube ").arg(binding);
        } else {
            texture += QString("layout(binding = %1) uniform sampler2D ").arg(binding);
        }
        texture += ((it.second & ShaderRootNode::Target) ? it.first : QString("texture%1").arg(i)) + ";\n";
        layout.append(texture);

        i++;
        binding++;
    }

    layout.append("\n");

    addPragma("uniforms", layout.toStdString());

    QString vertex, fragment;

    for(int i = 0; i < m_inputs.size(); i++) {
        if(std::next(m_inputs.begin(), i)->m_vertex) {
            vertex.append(functions.at(i));
        } else {
            fragment.append(functions.at(i));
        }
    }

    addPragma("vertex", vertex.toStdString());
    addPragma("fragment", fragment.toStdString());

    return true;
}

VariantMap ShaderNodeGraph::data(bool editor) const {
    ShaderRootNode *root = static_cast<ShaderRootNode *>(m_rootNode);

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
    user[TEXTURES] = textures;

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
    if(root->materialType() == ShaderRootNode::PostProcess) {
        define = "#define TYPE_FULLSCREEN 1";
    } else {
        define = "#define TYPE_STATIC 1";
    }
    {
        Variant data = ShaderBuilder::loadIncludes(vertex, define, m_pragmas).toStdString();
        if(data.isValid()) {
            user[STATIC] = data;
        }
    }
    if(root->materialType() == ShaderRootNode::Surface && !editor) {
        {
            define += "\n#define INSTANCING 1";
            Variant data = ShaderBuilder::loadIncludes(vertex, define, m_pragmas).toStdString();
            if(data.isValid()) {
                user[INSTANCED] = data;
            }
        }
        {
            Variant data = ShaderBuilder::loadIncludes(vertex, "#define TYPE_BILLBOARD 1", m_pragmas).toStdString();
            if(data.isValid()) {
                user[PARTICLE] = data;
            }
        }
        {
            Variant data = ShaderBuilder::loadIncludes(vertex, "#define TYPE_SKINNED 1", m_pragmas).toStdString();
            if(data.isValid()) {
                user[SKINNED] = data;
            }
        }
    }

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

QStringList ShaderNodeGraph::buildFrom(GraphNode *node) {
    for(auto &it : m_nodes) {
        ShaderFunction *node = dynamic_cast<ShaderFunction *>(it);
        if(node) {
            node->reset();
        }
    }

    int32_t depth = 0;

    QStringList result;
    for(NodePort &port : node->ports()) { // Iterate all ports for the node
        if(port.m_out == false) {
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

            QString function;
            const Link *link = findLink(node, &port);
            if(link) {
                ShaderFunction *node = dynamic_cast<ShaderFunction *>(link->sender);
                if(node) {
                    QStack<QString> stack;
                    int32_t size = 0;
                    int32_t index = node->build(function, stack, this, *link, depth, size);

                    if(index >= 0) {
                        if(stack.isEmpty()) {
                            function.append(QString("%1 = %2;\n").arg(type, ShaderFunction::convert("local" + QString::number(index), size, port.m_type)));
                        } else {
                            function.append(QString("%1 = %2;\n").arg(type, ShaderFunction::convert(stack.pop(), size, port.m_type)));
                        }
                    }
                }
            } else { // Default value
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
            result << function;
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
