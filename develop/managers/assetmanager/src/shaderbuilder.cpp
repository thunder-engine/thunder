#include "shaderbuilder.h"

#include <QFileInfo>
#include <QDebug>
#include <QMetaMethod>

#include <resources/texture.h>

#include <file.h>
#include <log.h>
#include <json.h>

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QDir>

#include "material/aconstvalue.h"
#include "material/acoordinates.h"
//#include "material/agradient.h"
#include "material/amaterialparam.h"
#include "material/amathfunction.h"
#include "material/amathoperator.h"
#include "material/atexturesample.h"
#include "material/autils.h"

#include "projectmanager.h"
#include "platforms/android.h"
#include "platforms/ios.h"

#include "functionmodel.h"
#include "spirvconverter.h"
#include "resources/text.h"

#include <regex>
#include <sstream>

#include <bson.h>

#define TYPE        "Type"
#define BLEND       "Blend"
#define MODEL       "Model"
#define SIDE        "Side"
#define DEPTH       "Depth"
#define RAW         "Raw"
#define VIEW        "View"

#define UNIFORM     50

#define FORMAT_VERSION 1

const regex include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">][^?]*");
const regex pragma("^[ ]*#[ ]*pragma[ ]+(.*)[^?]*");

ShaderBuilder::ShaderBuilder() :
        m_BlendMode(Opaque),
        m_LightModel(Lit),
        m_MaterialType(Surface),
        m_DoubleSided(false),
        m_DepthTest(true),
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

    QStringList list;
    list << "Diffuse" << "Emissive" << "Normal" << "Metallic" << "Roughness" << "Opacity" << "IOR";

    QVariantList value;
    value << QVector3D(1.0, 1.0, 1.0) << QVector3D(0.0, 0.0, 0.0) << QVector3D(0.5, 0.5, 1.0) << 0.0 << 0.0 << 1.0 << 1.0;

    int i   = 0;
    foreach(QString it, list) {
        Item *item  = new Item;
        item->name  = it;
        item->out   = false;
        item->pos   = i;
        item->type  = (i < 3) ? QMetaType::QVector3D : QMetaType::Double;
        item->var   = value.at(i);
        m_pRootNode->list.push_back(item);
        i++;
    }
}

ShaderBuilder::~ShaderBuilder() {
    cleanup();
}

IConverterSettings *ShaderBuilder::createSettings() const {
    IConverterSettings *result = AbstractSchemeModel::createSettings();
    result->setVersion(FORMAT_VERSION);
    return result;
}

uint8_t ShaderBuilder::convertFile(IConverterSettings *settings) {
    load(settings->source());
    if(build()) {
        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( object() );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            settings->setCurrentVersion(settings->version());
            return 0;
        }
    }

    return 1;
}

AbstractSchemeModel::Node *ShaderBuilder::nodeCreate(const QString &path, int &index) {
    const QByteArray className = qPrintable(path + "*");
    const int type = QMetaType::type( className );
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        QObject *object = meta->newInstance();
        ShaderFunction *function   = dynamic_cast<ShaderFunction *>(object);
        if(object && function) {
            connect(function, SIGNAL(updated()), this, SIGNAL(schemeUpdated()));
            Node *result    = function->createNode(this, path);
            result->type    = path;

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

QAbstractItemModel *ShaderBuilder::components() const {
    return new FunctionModel(m_Functions);
}

void ShaderBuilder::load(const QString &path) {
    blockSignals(true);
    AbstractSchemeModel::load(path);

    m_pRootNode->name = QFileInfo(path).baseName();

    setMaterialType(static_cast<Type>(m_Data[TYPE].toInt()));
    setBlend(static_cast<Blend>(m_Data[BLEND].toInt()));
    setLightModel(static_cast<LightModel>(m_Data[MODEL].toInt()));
    setDoubleSided(m_Data[SIDE].toBool());
    setDepthTest(m_Data.contains(DEPTH) ? m_Data[DEPTH].toBool() : true);
    setRawPath(m_Data[RAW].toString());
    blockSignals(false);

    emit schemeUpdated();
}

void ShaderBuilder::save(const QString &path) {
    m_Data[TYPE] = materialType();
    m_Data[BLEND] = blend();
    m_Data[MODEL] = lightModel();
    m_Data[SIDE] = isDoubleSided();
    m_Data[DEPTH] = isDepthTest();
    m_Data[VIEW] = isViewSpace();
    m_Data[RAW] = rawPath().filePath();

    AbstractSchemeModel::save(path);
}

void ShaderBuilder::loadUserValues(Node *node, const QVariantMap &values) {
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

void ShaderBuilder::saveUserValues(Node *node, QVariantMap &values) {
    QObject *func = static_cast<QObject *>(node->ptr);
    if(func) {
        const QMetaObject *meta   = func->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property  = meta->property(i);
            if(property.isUser(func)) {
                QVariant value  = property.read(func);
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
                        QColor col      = value.value<QColor>();
                        v.push_back(col.red());
                        v.push_back(col.green());
                        v.push_back(col.blue());
                        v.push_back(col.alpha());
                        values[property.name()] = v;
                    } break;
                    default: {
                        if(value.canConvert<Template>()) {
                            Template tmp    = value.value<Template>();
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

bool ShaderBuilder::build() {
    cleanup();

    // Nodes
    QString str;
    buildRoot(str);

    if(!m_Uniforms.empty() || !m_Textures.empty()) {
        m_Shader = QString("layout(location = %1) uniform struct Uniforms {\n").arg(UNIFORM);
    }
    // Make uniforms
    for(const auto &it : m_Uniforms) {
        switch(it.second.first) {
            case QMetaType::QVector2D:  m_Shader += "\tvec2 " + it.first + ";\n"; break;
            case QMetaType::QVector3D:  m_Shader += "\tvec3 " + it.first + ";\n"; break;
            case QMetaType::QVector4D:  m_Shader += "\tvec4 " + it.first + ";\n"; break;
            default:  m_Shader += "\tfloat " + it.first + ";\n"; break;
        }
    }
    m_Shader.append("\n");
    // Textures
    uint16_t i  = 0;
    for(auto it : m_Textures) {
        QString texture;
        if(it.second & Cube) {
            texture += "\tsamplerCube ";
        } else {
            texture += "\tsampler2D ";
        }
        texture += ((it.second & Target) ? it.first : QString("texture%1").arg(i)) + ";\n";
        m_Shader.append( texture );
        i++;
    }
    if(!m_Uniforms.empty() || !m_Textures.empty()) {
        m_Shader += "} uni;\n";
    }
    m_Shader.append(str);

    addPragma("version", "#version 450 core");
    addPragma("material", m_Shader.toStdString());

    return true;
}

Variant ShaderBuilder::object() const {
    VariantList result;

    VariantList object;

    object.push_back(Material::metaClass()->name()); // type
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(Material::metaClass()->name()); // name

    object.push_back(VariantMap()); // properties

    object.push_back(VariantList()); // links
    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}

Variant ShaderBuilder::data(bool editor) const {
    VariantMap user;
    VariantList properties;
    properties.push_back(materialType());
    properties.push_back(isDoubleSided());
    properties.push_back((materialType() == ShaderBuilder::Surface) ?
                             (Material::Static | Material::Skinned | Material::Billboard | Material::Oriented) :
                             Material::Static );
    properties.push_back(blend());
    properties.push_back(lightModel());
    properties.push_back(isDepthTest());
    user["Properties"]  = properties;

    VariantMap textures;
    uint16_t i  = 0;
    for(auto it : m_Textures) {
        bool target = (it.second & Target);
        QString name = QString("uni.%1").arg((target) ? it.first : QString("texture%1").arg(i));
        textures[name.toStdString()] = ((target) ? "" : it.first.toStdString());
        i++;
    }
    user["Textures"] = textures;

    VariantMap uniforms;
    for(auto it : m_Uniforms) {
        Variant value;
        switch(it.second.first) {
            case QMetaType::QVector4D: {
                QColor c = it.second.second.value<QColor>();
                value = Variant(Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
            } break;
            default: {
                value = Variant(it.second.second.toFloat());
            } break;
        }
        uniforms[string("uni.") + it.first.toStdString()] = value;
    }
    user["Uniforms"] = uniforms;

    string define;
    switch(m_BlendMode) {
        case Additive: {
            define  = "#define BLEND_ADDITIVE 1";
        } break;
        case Translucent: {
            define  = "#define BLEND_TRANSLUCENT 1";
        } break;
        default: {
            define  = "#define BLEND_OPAQUE 1";
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

    uint32_t version = 430;
    bool es = false;
    IPlatform *platform = ProjectManager::instance()->currentPlatform();
    if(dynamic_cast<AndroidPlatform *>(platform) != nullptr ||
       dynamic_cast<IOSPlatform *>(platform) != nullptr) {
        version = 300;
        es = true;
    }
    SpirVConverter::setGlslVersion(version, es);

    QString fragment = (!m_RawPath.filePath().isEmpty()) ? m_RawPath.filePath() : "Surface.frag";
    {
        QString buff = loadIncludes(fragment, define);
        if(editor) {
            user["Shader"] = buff.toStdString();
        } else {
            vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangFragment);
            if(!spv.empty()) {
                user["Shader"] = SpirVConverter::spvToGlsl(spv);
            }
        }
    }
    if(m_MaterialType == Surface && !editor) {
        define += "\n#define SIMPLE 1";
        QString buff = loadIncludes(fragment, define);
        vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangFragment);
        if(!spv.empty()) {
            user["Simple"] = SpirVConverter::spvToGlsl(spv);
        }
    }

    QString vertex = "BasePass.vert";
    define = "#define TYPE_STATIC 1";
    {
        QString buff = loadIncludes(vertex, define);
        if(editor) {
            user["Static"] = buff.toStdString();
        } else {
            vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangVertex);
            if(!spv.empty()) {
                user["Static"] = SpirVConverter::spvToGlsl(spv);
            }
        }
    }
    if(m_MaterialType == Surface && !editor) {
        {
            define += "\n#define INSTANCING 1";
            QString buff = loadIncludes(vertex, define);
            vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangVertex);
            if(!spv.empty()) {
                user["StaticInst"] = SpirVConverter::spvToGlsl(spv);
            }
        }
        {
            QString buff = loadIncludes(vertex, "#define TYPE_BILLBOARD 1");
            vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangVertex);
            if(!spv.empty()) {
                user["Particle"] = SpirVConverter::spvToGlsl(spv);
            }
        }
        {
            QString buff = loadIncludes(vertex, "#define TYPE_SKINNED 1");
            vector<uint32_t> spv = SpirVConverter::glslToSpv(buff.toStdString(), EShLanguage::EShLangVertex);
            if(!spv.empty()) {
                user["Skinned"] = SpirVConverter::spvToGlsl(spv);
            }
        }
    }

    return user;
}

int ShaderBuilder::setTexture(const QString &path, Vector4 &sub, uint8_t flags) {
    sub     = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index   = m_Textures.indexOf(TexturePair(path, flags));
    if(index == -1) {
        index   = m_Textures.size();
        m_Textures.push_back(TexturePair(path, flags));
    }
    return index;
}

void ShaderBuilder::addUniform(const QString &name, uint8_t type, const QVariant &value) {
    m_Uniforms[name] = UniformPair(type, value);
}

void ShaderBuilder::addParam(const QString &param) {
    m_Params.append(param).append("\n");
}

void ShaderBuilder::buildRoot(QString &result) {
    for(const auto it : m_pRootNode->list) {
        for(auto it : m_Nodes) {
            ShaderFunction *node   = static_cast<ShaderFunction *>(it->ptr);
            if(node && it->ptr != this) {
                node->reset();
            }
        }

        Item *item  = it;
        switch(item->type) {
            case QMetaType::Double:     result += "float get" + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector2D:  result += "vec2 get"  + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector3D:  result += "vec3 get"  + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector4D:  result += "vec4 get"  + item->name + "(Params p) {\n"; break;
            default: break;
        }

        const Link *link = findLink(m_pRootNode, qPrintable(item->name));
        if(link) {
            ShaderFunction *node = static_cast<ShaderFunction *>(link->sender->ptr);
            if(node) {
                int32_t depth = 0;
                uint8_t size = 0;
                int32_t index = node->build(result, *link, depth, size);
                if(index >= 0) {
                    result += "\treturn " + ShaderFunction::convert("local" + QString::number(index), size, item->type) + ";\n";
                    result.append("}\n\n");
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

        result.append(data);
        result.append("}\n");
    }
}

void ShaderBuilder::cleanup() {
    m_Textures.clear();
    m_Params.clear();
    m_Uniforms.clear();
    m_Pragmas.clear();
    m_Shader.clear();
}

void ShaderBuilder::addPragma(const string &key, const string &value) {
    m_Pragmas[key]  = m_Pragmas[key].append(value).append("\r\n");
}

QString ShaderBuilder::loadIncludes(const QString &path, const string &define) const {
    QString output;

    QStringList paths;
    paths << ProjectManager::instance()->contentPath() + "/";
    paths << ProjectManager::instance()->resourcePath() + "/engine/shaders/";
    paths << ProjectManager::instance()->resourcePath() + "/editor/shaders/";

    foreach(QString it, paths) {
        QFile file(it + path);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while(!file.atEnd()) {
                QString line = file.readLine();
                smatch matches;
                string data = line.toStdString();
                if(regex_match(data, matches, include)) {
                    string next(matches[1]);
                    output += loadIncludes(next.c_str(), define) + "\n";
                } else if(regex_match(data, matches, pragma)) {
                    if(matches[1] == "flags") {
                        output += QString(define.c_str()) + "\n";
                    } else {
                        auto it = m_Pragmas.find(matches[1]);
                        if(it != m_Pragmas.end()) {
                            output += QString(m_Pragmas.at(matches[1]).c_str()) + "\n";
                        }
                    }
                } else {
                    output += line + "\n";
                }

            }
            file.close();

            return output;
        }
    }

    return output;
}
