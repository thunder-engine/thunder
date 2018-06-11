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

#include "projectmanager.h"

#include "functionmodel.h"

#define NODES       "Nodes"
#define LINKS       "Links"
#define TYPE        "Type"
#define VALUES      "Values"
#define SENDER      "Sender"
#define RECEIVER    "Receiver"
#define SITEM       "SItem"
#define RITEM       "RItem"
#define BLEND       "Blend"
#define MODEL       "Model"
#define SIDE        "Side"
#define DEPTH       "Depth"
#define X           "X"
#define Y           "Y"

ShaderBuilder::ShaderBuilder() :
        m_DoubleSided(false),
        m_DepthTest(true),
        m_BlendMode(Opaque),
        m_LightModel(Lit),
        m_MaterialType(Surface),
        m_Tangent(false) {

    qRegisterMetaType<ConstFloat*>("ConstFloat");
    qRegisterMetaType<ConstVector2*>("ConstVector2");
    qRegisterMetaType<ConstVector3*>("ConstVector3");
    qRegisterMetaType<ConstVector4*>("ConstVector4");
    m_Functions << "ConstFloat" << "ConstVector2" << "ConstVector3" << "ConstVector4";

    qRegisterMetaType<TexCoord*>("TexCoord");
    qRegisterMetaType<CoordPanner*>("CoordPanner");
    m_Functions << "TexCoord" << "CoordPanner";

    qRegisterMetaType<ParamFloat*>("ParamFloat");
    qRegisterMetaType<ParamVector*>("ParamVector");
    m_Functions << "ParamFloat" << "ParamVector";

    qRegisterMetaType<TextureSample*>("TextureSample");
    qRegisterMetaType<TextureSampleCube*>("TextureSampleCube");
    m_Functions << "TextureSample" << "TextureSampleCube";

    qRegisterMetaType<DotProduct*>("DotProduct");
    qRegisterMetaType<CrossProduct*>("CrossProduct");
    qRegisterMetaType<Clamp*>("Clamp");
    qRegisterMetaType<Mod*>("Mod");
    qRegisterMetaType<Abs*>("Abs");
    qRegisterMetaType<Floor*>("Floor");
    qRegisterMetaType<Ceil*>("Ceil");
    qRegisterMetaType<Sine*>("Sine");
    qRegisterMetaType<Cosine*>("Cosine");
    qRegisterMetaType<Tangent*>("Tangent");
    qRegisterMetaType<ArcSine*>("ArcSine");
    qRegisterMetaType<ArcCosine*>("ArcCosine");
    qRegisterMetaType<ArcTangent*>("ArcTangent");
    m_Functions << "DotProduct" << "CrossProduct" << "Clamp" << "Mod" << "Abs" << "Floor" << "Ceil";
    m_Functions << "Sine" << "Cosine" << "Tangent" << "ArcSine" << "ArcCosine" << "ArcTangent";

    qRegisterMetaType<Subtraction*>("Subtraction");
    qRegisterMetaType<Add*>("Add");
    qRegisterMetaType<Divide*>("Divide");
    qRegisterMetaType<Multiply*>("Multiply");
    m_Functions << "Subtraction" << "Add" << "Divide" << "Multiply";

    m_pNode = new Node;
    m_pNode->root   = true;
    m_pNode->name   = "";
    m_pNode->ptr    = this;

    QStringList list;
    list << "Diffuse" << "Emissive" << "Normal" << "Metallic" << "Roughness" << "Opacity" << "IOR";

    QVariantList value;
    value << QVector3D(1.0, 1.0, 1.0) << QVector3D(0.0, 0.0, 0.0) << QVector3D(0.0, 0.0, 1.0) << 0.0 << 0.0 << 1.0 << 1.0;

    int i   = 0;
    foreach(QString it, list) {
        Item *item  = new Item;
        item->name  = it;
        item->out   = false;
        item->pos   = i;
        item->type  = (i < 3) ? QMetaType::QVector3D : QMetaType::Double;
        item->var   = value.at(i);
        m_pNode->list.push_back(item);
        i++;
    }
    m_Nodes.push_back(m_pNode);
}

ShaderBuilder::~ShaderBuilder() {
    cleanup();
}

AbstractSchemeModel::Node *ShaderBuilder::createNode(const QString &path) {
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
            m_Nodes.push_back(result);

            return result;
        }
    }

    return nullptr;
}

void ShaderBuilder::deleteNode(Node *node) {
    bool result = false;
    AbstractSchemeModel::NodeList::iterator it  = m_Nodes.begin();
    while(it != m_Nodes.end()) {
        if(*it == node) {
            for(Item *it : node->list) {
                deleteLink(it, true);
                delete it;
            }
            it  = m_Nodes.erase(it);

            result  = true;
        } else {
            ++it;
        }
    }
    if(result) {
        emit schemeUpdated();
    }
}

void ShaderBuilder::createLink(Node *sender, Item *sitem, Node *receiver, Item *ritem) {
    bool result     = true;
    for(auto it : m_Links) {
        if(it->sender == sender && it->receiver == receiver && it->sitem == sitem && it->ritem == ritem) {
            result  = false;
            break;
        }
    }
    if(result) {
        for(auto it : m_Links) {
            if(it->ritem == ritem) {
                deleteLink(ritem);
            }
        }

        Link *link      = new Link;
        link->sender    = sender;
        link->receiver  = receiver;
        link->sitem     = sitem;
        link->ritem     = ritem;
        m_Links.push_back(link);

        emit schemeUpdated();
    }
}

void ShaderBuilder::deleteLink(Item *item, bool silent) {
    bool result = false;
    AbstractSchemeModel::LinkList::iterator it  = m_Links.begin();
    while(it != m_Links.end()) {
        AbstractSchemeModel::Link *link   = *it;
        if(link->sitem == item || link->ritem == item) {
            it  = m_Links.erase(it);
            result  = true;
        } else {
            ++it;
        }
    }
    if(result && !silent) {
        emit schemeUpdated();
    }
}

QAbstractItemModel *ShaderBuilder::components() const {
    return new FunctionModel(m_Functions);
}

void ShaderBuilder::load(const QString &path) {
    foreach(Link *it, m_Links) {
        delete it;
    }
    m_Links.clear();

    foreach(Node *it, m_Nodes) {
        if(it != m_pNode) {
            delete it;
        }
    }
    m_Nodes.clear();

    m_pNode->name  = QFileInfo(path).baseName();
    m_Nodes.push_back(m_pNode);

    QFile loadFile(path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(saveData));

    QJsonObject json   = doc.object();
    QJsonArray nodes    = json[NODES].toArray();
    for(int i = 0; i < nodes.size(); ++i) {
        QJsonObject n   = nodes[i].toObject();
        Node *node  = createNode(n[TYPE].toString());
        node->pos   = QPoint(n[X].toInt(), n[Y].toInt());
        QObject *obj    = static_cast<QObject *>(node->ptr);
        if(obj) {
            obj->blockSignals(true);
            QJsonObject values  = n[VALUES].toObject();
            foreach(QString key, values.keys()) {
                if(values[key].isString()) {
                    obj->setProperty(qPrintable(key), values[key].toString());
                } else if(values[key].isDouble()) {
                    obj->setProperty(qPrintable(key), values[key].toDouble());
                } else if(values[key].isArray()) {
                    QJsonArray array    = values[key].toArray();
                    switch(array.first().toInt()) {
                        case QVariant::Color: {
                            obj->setProperty(qPrintable(key), QColor(array.at(1).toInt(), array.at(2).toInt(), array.at(3).toInt(), array.at(4).toInt()));
                        } break;
                        default: {
                            if(array.first().toString() == "Template") {
                                obj->setProperty(qPrintable(key), QVariant::fromValue(Template(array.at(1).toString(), (IConverter::ContentTypes)array.at(2).toInt())));
                            }
                        } break;
                    }
                }
            }
            obj->blockSignals(false);
        }
    }

    blockSignals(true);
    QJsonArray links    = json[LINKS].toArray();
    for(int i = 0; i < links.size(); ++i) {
        QJsonObject l   = links[i].toObject();
        Node *sender    = m_Nodes.at(l[SENDER].toInt());
        Node *receiver  = m_Nodes.at(l[RECEIVER].toInt());
        if(sender && receiver) {
            Item *sitem = nullptr;
            Item *ritem = nullptr;
            for(Item *it : sender->list) {
                if(it->name == l[SITEM].toString()) {
                    sitem   = it;
                    break;
                }
            }
            for(Item *it : receiver->list) {
                if(it->name == l[RITEM].toString()) {
                    ritem   = it;
                    break;
                }
            }
            if(sitem && ritem) {
                createLink(sender, sitem, receiver, ritem);
            }
        }
    }
    setMaterialType((Type)json[TYPE].toInt());
    setBlend((Blend)json[BLEND].toInt());
    setLightModel((LightModel)json[MODEL].toInt());
    setDoubleSided(json[SIDE].toBool());
    setDepthTest(json.contains(DEPTH) ? json[DEPTH].toBool() : true);
    blockSignals(false);

    emit schemeUpdated();
}

void ShaderBuilder::save(const QString &path) {
    QFile saveFile(path);
    if(!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    QJsonObject data;
    QJsonArray nodes;
    for(Node *it : m_Nodes) {
        if(it != m_pNode) {
            QJsonObject node;
            node[TYPE]  = it->type;
            node[X]     = it->pos.x();
            node[Y]     = it->pos.y();

            QObject *func   = static_cast<QObject *>(it->ptr);
            if(func) {
                QJsonObject values;
                const QMetaObject *meta   = func->metaObject();
                for(int i = 0; i < meta->propertyCount(); i++) {
                    QMetaProperty property  = meta->property(i);
                    if(property.isUser(func)) {
                        QVariant value  = property.read(func);
                        switch(value.type()) {
                            case QVariant::String: {
                                values[property.name()] = value.toString();
                            } break;
                            case QVariant::Double: {
                                values[property.name()] = value.toDouble();
                            } break;
                            case QVariant::Color: {
                                QJsonArray v;
                                v.push_back((int32_t)QVariant::Color);
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
                                    v.push_back(tmp.type);
                                    values[property.name()] = v;
                                }
                            } break;
                        }
                    }
                }
                node[VALUES]    = values;
            }
            nodes.push_back(node);
        }
    }
    data[NODES] = nodes;

    QJsonArray links;
    for(Link *it : m_Links) {
        int s   = -1;
        for(int i = 0; i < m_Nodes.size(); i++) {
            if(m_Nodes.at(i) == it->sender) {
                s   = i;
                break;
            }
        }
        int r   = - 1;
        for(int i = 0; i < m_Nodes.size(); i++) {
            if(m_Nodes.at(i) == it->receiver) {
                r   = i;
                break;
            }
        }
        if(s > -1 && r > -1) {
            QJsonObject link;
            link[SENDER]    = s;
            link[SITEM]     = it->sitem->name;
            link[RECEIVER]  = r;
            link[RITEM]     = it->ritem->name;
            links.push_back(link);
        }
    }
    data[LINKS] = links;
    data[TYPE]  = materialType();
    data[BLEND] = blend();
    data[MODEL] = lightModel();
    data[SIDE]  = isDoubleSided();
    data[DEPTH] = isDepthTest();

    QJsonDocument doc(data);
    saveFile.write(doc.toJson());
}

bool ShaderBuilder::build() {
    cleanup();

    m_Shader.clear();
    // Nodes
    QString str;
    buildRoot(str);
    // Make uniforms
    for(const auto &it : m_Uniforms) {
        switch(it.second) {
            case QMetaType::QVector2D:  m_Shader += "uniform vec2 " + it.first + ";\n"; break;
            case QMetaType::QVector3D:  m_Shader += "uniform vec3 " + it.first + ";\n"; break;
            case QMetaType::QVector4D:  m_Shader += "uniform vec4 " + it.first + ";\n"; break;
            default:  m_Shader += "uniform float " + it.first + ";\n"; break;
        }
    }
    m_Shader.append("\n");
    // Textures
    uint16_t i  = 0;
    for(auto it : m_Textures) {
        QString texture = "uniform ";
        if(it.second) {
            texture += "samplerCube";
        } else {
            texture += "sampler2D";
        }
        texture += " texture" + QString::number(i) + ";\n";
        m_Shader.append( texture );
        i++;
    }
    m_Shader.append("\n");
    m_Shader.append(str);
    // Options
    m_Tangent   = findLink(m_pNode, "Normal");
    return true;
}

Variant ShaderBuilder::object() const {
    VariantList result;

    VariantList object;

    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(Material::metaClass()->name()); // type
    object.push_back(Material::metaClass()->name()); // name

    object.push_back(VariantMap()); // properties

    object.push_back(data()); // user data
    object.push_back(VariantList()); // links

    result.push_back(object);

    return result;
}

Variant ShaderBuilder::data() const {
    VariantMap user;
    VariantList properties;
    properties.push_back(materialType());
    properties.push_back(isDoubleSided());
    properties.push_back(isTangent());
    properties.push_back((materialType() == Material::Surface) ? (Material::Static | Material::Skinned | Material::Billboard | Material::Oriented) : Material::Static );
    properties.push_back(blend());
    properties.push_back(lightModel());
    properties.push_back(isDepthTest());

    VariantMap textures;
    uint16_t i  = 0;
    for(auto it : m_Textures) {
        textures["texture" + to_string(i)]  = it.first.toStdString();
        i++;
    }
    user["Textures"]    = textures;
    user["Properties"]  = properties;
    user["Shader"]      = shader().toStdString();

    return user;
}

int ShaderBuilder::setTexture(const QString &path, Vector4 &sub, bool cube) {
    sub     = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    int index   = m_Textures.indexOf(TexturePair(path, cube));
    if(index == -1) {
        m_Textures.push_back(TexturePair(path, cube));
        return m_Textures.size() - 1;
    }
    return index;
}

void ShaderBuilder::addUniform(const QString &name, uint8_t type) {
    m_Uniforms[name] = type;
}

void ShaderBuilder::addParam(const QString &param) {
    m_Params.append(param).append("\n");
}

void ShaderBuilder::buildRoot(QString &result) {
    for(const auto it : m_pNode->list) {
        Item *item  = it;
        switch(item->type) {
            case QMetaType::Double:     result += "float get" + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector2D:  result += "vec2 get"  + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector3D:  result += "vec3 get"  + item->name + "(Params p) {\n"; break;
            case QMetaType::QVector4D:  result += "vec4 get"  + item->name + "(Params p) {\n"; break;
            default: break;
        }

        const Link *link    = findLink(m_pNode, qPrintable(item->name));
        if(link) {
            ShaderFunction *node   = static_cast<ShaderFunction *>(link->sender->ptr);
            if(node) {
                uint32_t depth  = 0;
                uint8_t size    = 0;
                if(node->build(result, *link, depth, size)) {
                    result  += "\treturn " + ShaderFunction::convert("local" + QString::number(depth), size, item->type) + ";\n";
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
}
