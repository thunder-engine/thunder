#include "shaderschememodel.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QMetaProperty>

#include <sstream>

#include <editor/projectmanager.h>

#include "functions/constvalue.h"
#include "functions/coordinates.h"
//#include "functions/gradient.h"
#include "functions/materialparam.h"
#include "functions/mathfunction.h"
#include "functions/mathoperator.h"
#include "functions/texturesample.h"
#include "functions/utils.h"

#include "shaderbuilder.h"

ShaderSchemeModel::ShaderSchemeModel() :
        m_BlendMode(Opaque),
        m_LightModel(Lit),
        m_MaterialType(Surface),
        m_DoubleSided(false),
        m_DepthTest(true),
        m_DepthWrite(true),
        m_ViewSpace(true) {

    qRegisterMetaType<ConstFloat*>("ConstFloat");
    qRegisterMetaType<ConstVector2*>("ConstVector2");
    qRegisterMetaType<ConstVector3*>("ConstVector3");
    qRegisterMetaType<ConstVector4*>("ConstVector4");
    m_Functions << "ConstFloat" << "ConstVector2" << "ConstVector3" << "ConstVector4";

    qRegisterMetaType<TexCoord*>("TexCoord");
    qRegisterMetaType<NormalVectorWS*>("NormalVectorWS");
    qRegisterMetaType<CameraPosition*>("CameraPosition");
    qRegisterMetaType<CameraDirection*>("CameraDirection");
    qRegisterMetaType<ProjectionCoord*>("ProjectionCoord");
    qRegisterMetaType<CoordPanner*>("CoordPanner");
    m_Functions << "TexCoord" << "NormalVectorWS" << "CameraPosition" << "CameraDirection" << "ProjectionCoord" << "CoordPanner";

    qRegisterMetaType<ParamFloat*>("ParamFloat");
    qRegisterMetaType<ParamVector*>("ParamVector");
    m_Functions << "ParamFloat" << "ParamVector";

    qRegisterMetaType<TextureSample*>("TextureSample");
    qRegisterMetaType<RenderTargetSample*>("RenderTargetSample");
    qRegisterMetaType<TextureSampleCube*>("TextureSampleCube");
    m_Functions << "TextureSample" << "RenderTargetSample" << "TextureSampleCube";

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
    m_Functions << "DotProduct" << "CrossProduct" << "Mix" << "Smoothstep" << "Mod" << "Power" << "SquareRoot" << "Logarithm" << "Logarithm2" << "FWidth";

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
    m_Functions << "Clamp" << "Min" << "Max" << "Abs" << "Sign" << "Floor" << "Ceil" << "Round" << "Truncate" << "Fract" << "Normalize";

    qRegisterMetaType<Sine*>("Sine");
    qRegisterMetaType<Cosine*>("Cosine");
    qRegisterMetaType<Tangent*>("Tangent");
    qRegisterMetaType<ArcSine*>("ArcSine");
    qRegisterMetaType<ArcCosine*>("ArcCosine");
    qRegisterMetaType<ArcTangent*>("ArcTangent");
    m_Functions << "Sine" << "Cosine" << "Tangent" << "ArcSine" << "ArcCosine" << "ArcTangent";

    qRegisterMetaType<Subtraction*>("Subtraction");
    qRegisterMetaType<Add*>("Add");
    qRegisterMetaType<Divide*>("Divide");
    qRegisterMetaType<Multiply*>("Multiply");
    m_Functions << "Subtraction" << "Add" << "Divide" << "Multiply";

    qRegisterMetaType<Mask*>("Mask");
    qRegisterMetaType<If*>("If");
    m_Functions << "Mask" << "If";

    m_Inputs.append({ "Diffuse",                QVector3D(1.0, 1.0, 1.0), false });
    m_Inputs.append({ "Emissive",               QVector3D(0.0, 0.0, 0.0), false });
    m_Inputs.append({ "Normal",                 QVector3D(0.5, 0.5, 1.0), false });
    m_Inputs.append({ "Metallic",               0.0, false });
    m_Inputs.append({ "Roughness",              0.0, false });
    m_Inputs.append({ "Opacity",                1.0, false });
    m_Inputs.append({ "IOR",                    1.0, false });
    m_Inputs.append({ "World Position Offset",  QVector3D(0.0, 0.0, 0.0), true });

    int i = 0;
    foreach(auto it, m_Inputs) {
        Port *item = new Port;
        item->name = it.m_name;
        item->out  = false;
        item->pos  = i;
        item->type = it.m_value.type();
        item->var  = it.m_value;
        m_pRootNode->list.push_back(item);

        i++;
    }
}

ShaderSchemeModel::~ShaderSchemeModel() {
    cleanup();
}

AbstractSchemeModel::Node *ShaderSchemeModel::nodeCreate(const QString &path, int &index) {
    const QByteArray className = qPrintable(path + "*");
    const int type = QMetaType::type( className );
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        QObject *object = meta->newInstance();
        ShaderFunction *function = dynamic_cast<ShaderFunction *>(object);
        if(object && function) {
            connect(function, SIGNAL(updated()), this, SIGNAL(schemeUpdated()));
            Node *result = function->createNode(this, path);
            result->type = path;

            if(index == -1) {
                index = m_Nodes.size();
                m_Nodes.push_back(result);
            } else {
                m_Nodes.insert(index, result);
            }
            return result;
        }
    }

    return nullptr;
}

QStringList ShaderSchemeModel::nodeList() const {
    QStringList result;
    for(auto &it : m_Functions) {
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

void ShaderSchemeModel::load(const QString &path) {
    blockSignals(true);
    AbstractSchemeModel::load(path);

    m_pRootNode->name = QFileInfo(path).baseName();

    setMaterialType(static_cast<Type>(m_Data[TYPE].toInt()));
    setBlend(static_cast<Blend>(m_Data[BLEND].toInt()));
    setLightModel(static_cast<LightModel>(m_Data[MODEL].toInt()));
    setDoubleSided(m_Data[SIDE].toBool());
    setDepthTest(m_Data.contains(DEPTH) ? m_Data[DEPTH].toBool() : true);
    setDepthWrite(m_Data.contains(DEPTHWRITE) ? m_Data[DEPTHWRITE].toBool() : true);
    setRawPath(m_Data[RAW].toString());
    loadTextures(m_Data[TEXTURES].toMap());
    loadUniforms(m_Data[UNIFORMS].toList());
    blockSignals(false);

    emit schemeUpdated();
}

void ShaderSchemeModel::save(const QString &path) {
    m_Data[TYPE] = materialType();
    m_Data[BLEND] = blend();
    m_Data[MODEL] = lightModel();
    m_Data[SIDE] = isDoubleSided();
    m_Data[DEPTH] = isDepthTest();
    m_Data[DEPTHWRITE] = isDepthWrite();
    m_Data[RAW] = rawPath().filePath();
    m_Data[TEXTURES] = saveTextures();
    m_Data[UNIFORMS] = saveUniforms();

    AbstractSchemeModel::save(path);
}

void ShaderSchemeModel::loadUserValues(Node *node, const QVariantMap &values) {
    QObject *func = static_cast<QObject *>(node->ptr);
    if(func) {
        func->blockSignals(true);
        foreach(QString key, values.keys()) {
            if(static_cast<QMetaType::Type>(values[key].type()) == QMetaType::QVariantList) {
                QVariantList array = values[key].toList();
                switch(array.first().toInt()) {
                    case QVariant::Color: {
                        func->setProperty(qPrintable(key), QColor(array.at(1).toInt(), array.at(2).toInt(), array.at(3).toInt(), array.at(4).toInt()));
                    } break;
                    default: {
                        if(array.first().toString() == "Template") {
                            func->setProperty(qPrintable(key), QVariant::fromValue(Template(array.at(1).toString(),
                                                                                            array.at(2).toUInt())));
                        }
                    } break;
                }
            } else {
                func->setProperty(qPrintable(key), values[key]);
            }
        }
        func->blockSignals(false);
    }
}

void ShaderSchemeModel::saveUserValues(Node *node, QVariantMap &values) {
    QObject *func = static_cast<QObject *>(node->ptr);
    if(func) {
        const QMetaObject *meta = func->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property  = meta->property(i);
            if(property.isUser(func)) {
                QVariant value = property.read(func);
                switch(value.type()) {
                    case QVariant::Bool: {
                        values[property.name()] = value.toBool();
                    } break;
                    case QVariant::String: {
                        values[property.name()] = value.toString();
                    } break;
                    case QVariant::Double: {
                        values[property.name()] = value.toDouble();
                    } break;
                    case QVariant::Color: {
                        QJsonArray v;
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
                            QJsonArray v;
                            v.push_back(value.typeName());
                            v.push_back(tmp.path);
                            v.push_back(QJsonValue::fromVariant(tmp.type));
                            values[property.name()] = v;
                        }
                    } break;
                }
            }
        }
    }
}

void ShaderSchemeModel::loadTextures(const QVariantMap &data) {
    m_Textures.clear();
    foreach(auto it, data.keys()) {
        m_Textures.push_back(TexturePair(it, data.value(it).toInt()));
    }
}

QVariantMap ShaderSchemeModel::saveTextures() const {
    QVariantMap result;
    for(auto &it : m_Textures) {
        result[it.first] = it.second;
    }
    return result;
}

void ShaderSchemeModel::loadUniforms(const QVariantList &data) {
    m_Uniforms.clear();
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
                m_Uniforms.push_back({name, type, count, 0});
            } break;
            case QMetaType::Float: {
                m_Uniforms.push_back({name, type, count, 0.0f});
            } break;
            case QMetaType::QVector2D: {
                 m_Uniforms.push_back({name, type, count, QVector2D()});
            } break;
            case QMetaType::QVector3D: {
                m_Uniforms.push_back({name, type, count, QVector3D()});
            } break;
            case QMetaType::QVector4D: {
                m_Uniforms.push_back({name, type, count, QColor()});
            } break;
            case QMetaType::QMatrix4x4: {
                m_Uniforms.push_back({name, type, count, QMatrix4x4()});
            } break;
            default: break;
        }
    }
}

QVariantList ShaderSchemeModel::saveUniforms() const {
    QVariantList result;

    if(m_RawPath.absoluteFilePath().isEmpty()) {
        return result;
    }

    for(auto &it : m_Uniforms) {
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
            case QMetaType::QMatrix4x4: {
                value.push_back((int)QMetaType::QMatrix4x4);
            } break;
            default: break;
        }
        value.push_back((uint32_t)it.count);
        result.push_back(value);
    }
    return result;
}

bool ShaderSchemeModel::buildGraph() {
    cleanup();

    // Nodes
    QStringList functions = buildRoot();

    QString layout;
    if(m_RawPath.absoluteFilePath().isEmpty()) {
        uint32_t binding = UNIFORM;
        if(!m_Uniforms.empty()) {
            layout += QString("layout(binding = %1) uniform Uniforms {\n").arg(binding);

            // Make uniforms
            foreach(const auto &it, m_Uniforms) {
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
        for(auto &it : m_Textures) {
            QString texture;
            if(it.second & Cube) {
                texture += QString("layout(binding = %1) uniform samplerCube ").arg(binding);
            } else {
                texture += QString("layout(binding = %1) uniform sampler2D ").arg(binding);
            }
            texture += ((it.second & Target) ? it.first : QString("texture%1").arg(i)) + ";\n";
            layout.append(texture);

            i++;
            binding++;
        }

        layout.append("\n");
    }
    QString fragment(layout);
    QString vertex(layout);

    for(int i = 0; i < m_Inputs.size(); i++) {
        if(m_Inputs.at(i).m_vertex) {
            vertex.append(functions.at(i));
        } else {
            fragment.append(functions.at(i));
        }
    }

    addPragma("vertex", vertex.toStdString());
    addPragma("fragment", fragment.toStdString());

    return true;
}

VariantMap ShaderSchemeModel::data(bool editor) const {
    VariantMap user;
    VariantList properties;
    properties.push_back(materialType());
    properties.push_back(isDoubleSided());
    properties.push_back((materialType() == ShaderSchemeModel::Surface) ?
                             (Material::Static | Material::Skinned | Material::Billboard | Material::Oriented) :
                             Material::Static);
    properties.push_back(blend());
    properties.push_back(lightModel());
    properties.push_back(isDepthTest());
    properties.push_back(isDepthWrite());
    user[PROPERTIES] = properties;

    VariantList textures;
    uint16_t i = 0;
    uint32_t binding = (m_Uniforms.isEmpty()) ? UNIFORM : UNIFORM + 1;
    for(auto &it : m_Textures) {
        VariantList data;

        bool target = (it.second & Target);
        QString name;
        if(m_RawPath.absoluteFilePath().isEmpty()) {
            name = (target) ? it.first : QString("texture%1").arg(i);
        } else {
            name = it.first;
        }

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
    for(auto &it : m_Uniforms) {
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
            case QMetaType::QMatrix4x4: {
                value = Variant(Matrix4());
                size = sizeof(Matrix4);
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
    switch(m_BlendMode) {
        case Additive: {
            define = "#define BLEND_ADDITIVE 1";
        } break;
        case Translucent: {
            define = "#define BLEND_TRANSLUCENT 1";
        } break;
        default: {
            define = "#define BLEND_OPAQUE 1";
        } break;
    }
    switch(m_LightModel) {
        case Lit: {
            define += "\n#define MODEL_LIT 1";
        } break;
        case Subsurface: {
            define += "\n#define MODEL_SUBSURFACE 1";
        } break;
        default: {
            define += "\n#define MODEL_UNLIT 1";
        } break;
    }

    QString fragment = (!m_RawPath.filePath().isEmpty()) ? m_RawPath.filePath() : "Shader.frag";
    {
        Variant data = ShaderBuilder::loadIncludes(fragment, define, m_Pragmas).toStdString();
        if(data.isValid()) {
            user[SHADER] = data;
        }
    }
    if(m_MaterialType == Surface && !editor) {
        define += "\n#define SIMPLE 1";
        Variant data = ShaderBuilder::loadIncludes(fragment, define, m_Pragmas).toStdString();
        if(data.isValid()) {
            user[SIMPLE] = data;
        }
    }

    QString vertex = "Shader.vert";
    define = "#define TYPE_STATIC 1";
    {
        Variant data = ShaderBuilder::loadIncludes(vertex, define, m_Pragmas).toStdString();
        if(data.isValid()) {
            user[STATIC] = data;
        }
    }
    if(m_MaterialType == Surface && !editor) {
        {
            define += "\n#define INSTANCING 1";
            Variant data = ShaderBuilder::loadIncludes(vertex, define, m_Pragmas).toStdString();
            if(data.isValid()) {
                user[INSTANCED] = data;
            }
        }
        {
            Variant data = ShaderBuilder::loadIncludes(vertex, "#define TYPE_BILLBOARD 1", m_Pragmas).toStdString();
            if(data.isValid()) {
                user[PARTICLE] = data;
            }
        }
        {
            Variant data = ShaderBuilder::loadIncludes(vertex, "#define TYPE_SKINNED 1", m_Pragmas).toStdString();
            if(data.isValid()) {
                user[SKINNED] = data;
            }
        }
    }

    return user;
}

int ShaderSchemeModel::setTexture(const QString &path, Vector4 &sub, uint8_t flags) {
    sub = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index = m_Textures.indexOf(TexturePair(path, flags));
    if(index == -1) {
        index = m_Textures.size();
        m_Textures.push_back(TexturePair(path, flags));
    }
    return index;
}

void ShaderSchemeModel::addUniform(const QString &name, uint8_t type, const QVariant &value) {
    for(auto &it : m_Uniforms) {
        if(it.name == name) {
            it.type = type;
            it.value = value;
            return;
        }
    }
    m_Uniforms.push_back({name, type, 1, value});
}

QStringList ShaderSchemeModel::buildRoot() {
    QStringList result;
    foreach(const auto it, m_pRootNode->list) { // Iterate all ports for the root node
        QString function;
        for(auto &it : m_Nodes) {
            ShaderFunction *node = static_cast<ShaderFunction *>(it->ptr);
            if(node && it->ptr != this) {
                node->reset();
            }
        }

        Port *item  = it;

        QString name = item->name.replace(" ", "");
        switch(item->type) {
            case QMetaType::Double:    function += "float get" + name + "(Params p) {\n"; break;
            case QMetaType::QVector2D: function += "vec2 get"  + name + "(Params p) {\n"; break;
            case QMetaType::QVector3D: function += "vec3 get"  + name + "(Params p) {\n"; break;
            case QMetaType::QVector4D: function += "vec4 get"  + name + "(Params p) {\n"; break;
            default: break;
        }

        const Link *link = findLink(m_pRootNode, qPrintable(name));
        if(link) {
            ShaderFunction *node = static_cast<ShaderFunction *>(link->sender->ptr);
            if(node) {
                int32_t depth = 0;
                uint8_t size = 0;
                int32_t index = node->build(function, *link, depth, size);
                if(index >= 0) {
                    function += "\treturn " + ShaderFunction::convert("local" + QString::number(index), size, item->type) + ";\n";
                    function.append("}\n\n");

                    result << function;
                    continue;
                }
            }
        }

        QString data;
        switch(item->type) {
            case QMetaType::Double: {
                data += "\treturn " + QString::number(item->var.toDouble()) + ";\n";
            } break;
            case QMetaType::QVector2D: {
                QVector2D v = item->var.value<QVector2D>();
                data += QString("\treturn vec2(%1, %2);\n").arg(v.x()).arg(v.y());
            } break;
            case QMetaType::QVector3D: {
                QVector3D v = item->var.value<QVector3D>();
                data += QString("\treturn vec3(%1, %2, %3);\n").arg(v.x()).arg(v.y()).arg(v.z());
            } break;
            case QMetaType::QVector4D: {
                QVector4D v = item->var.value<QVector4D>();
                data += QString("\treturn vec4(%1, %2, %3, %4);\n").arg(v.x()).arg(v.y()).arg(v.z()).arg(v.w());
            } break;
            default: break;
        }

        function.append(data);
        function.append("}\n");

        result << function;
    }
    return result;
}

void ShaderSchemeModel::cleanup() {
    if(m_RawPath.absoluteFilePath().isEmpty()) {
        m_Textures.clear();
        m_Uniforms.clear();
    }
    m_Pragmas.clear();
}

void ShaderSchemeModel::addPragma(const string &key, const string &value) {
    m_Pragmas[key] = m_Pragmas[key].append(value).append("\r\n");
}
