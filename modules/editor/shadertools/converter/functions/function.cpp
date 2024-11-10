#include "function.h"

class FunctionObserver : public Object {
    A_REGISTER(FunctionObserver, Object, Editor)

    A_METHODS(
        A_SLOT(FunctionObserver::onPreview)
    )

public:
    FunctionObserver() :
            m_node(nullptr) {

    }

    void setNode(ShaderNode *node) {
        m_node = node;
    }

    void onPreview() {
        m_node->switchPreview();
    }

    ShaderNode *m_node;

};

ShaderNode::ShaderNode() :
    m_observer(new FunctionObserver),
    m_preview(nullptr),
    m_previewBtn(nullptr) {

    m_observer->setNode(this);

    reset();
}

ShaderNode::~ShaderNode() {
    delete m_observer;
}

void ShaderNode::createParams() {
    int i = 0;
    for(auto &it : m_outputs) {
        m_ports.push_back(NodePort(this, true, it.second, i, it.first, m_portColors[it.second]));
        i++;
    }

    for(auto &it : m_inputs) {
        m_ports.push_back(NodePort(this, false, it.second, i, it.first, m_portColors[it.second]));
        i++;
    }
}

int32_t ShaderNode::build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
    Q_UNUSED(code)
    Q_UNUSED(link)

    if(type == 0) {
        type = link.oport->m_type;
    }

    if(m_position == -1) {
        m_position = depth;
        depth++;
    }
    return m_position;
}

int32_t ShaderNode::compile(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
    if(m_position == -1) {
        QStringList args = getArguments(code, stack, depth, type);

        if(args.size() == m_inputs.size()) {
            QString expr = makeExpression(args);
            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(type, depth, expr));
            }
        } else {
            m_graph->reportMessage(this, QString("Missing argument"));
            return m_position;
        }
    } else {
        type = m_type;
    }

    return ShaderNode::build(code, stack, link, depth, type);
}

QStringList ShaderNode::getArguments(QString &code, QStack<QString> &stack, int32_t &depth, int32_t &type) {
    QStringList result;

    for(const NodePort &it : m_ports) {
        if(it.m_out == true) {
            continue;
        }

        uint32_t defaultType = 0;
        QString value = defaultValue(it.m_name, defaultType);

        const AbstractNodeGraph::Link *l = m_graph->findLink(this, &it);
        if(l) {
            ShaderNode *node = static_cast<ShaderNode *>(l->sender);

            int32_t l_type = 0;
            int32_t index = node->build(code, stack, *l, depth, l_type);
            if(index >= 0) {
                type = getOutType(l_type, l);

                if(stack.isEmpty()) {
                    value = QString("local%1").arg(QString::number(index));
                } else {
                    value = stack.pop();
                }
            }
        }

        if(!value.isEmpty()) {
            result << value;
        }
    }

    return result;
}

QString ShaderNode::convert(const QString &value, uint32_t current, uint32_t target, uint8_t component) {
    QString prefix;
    QString suffix;

    const char *names[] = {".x", ".y", ".z", ".w"};

    switch(target) {
        case QMetaType::Int: {
            switch(current) {
            case QMetaType::Float:      { prefix = "int("; suffix = ")"; } break;
            case QMetaType::QVector2D:
            case QMetaType::QVector3D:
            case QMetaType::QVector4D:  { prefix = "int("; suffix = QString(names[component]) + ")"; } break;
            case QMetaType::QTransform:
            case QMetaType::QMatrix4x4: { prefix = "int("; suffix = QString("[0]") + QString(names[component]) + ")"; } break;
            case QMetaType::QImage:     { prefix = "int(texture("; suffix = ", _uv0).x)"; } break;
            default: break;
            }
        }
        case QMetaType::Float: {
            switch(current) {
            case QMetaType::Int:        { prefix = "float("; suffix = ")"; } break;
            case QMetaType::QVector2D:
            case QMetaType::QVector3D:
            case QMetaType::QVector4D:  { prefix = ""; suffix = names[component]; } break;
            case QMetaType::QTransform:
            case QMetaType::QMatrix4x4: { prefix = ""; suffix = QString("[0]") + names[component]; } break;
            case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).x"; } break;
            default: break;
            }
        } break;
        case QMetaType::QVector2D: {
            switch(current) {
            case QMetaType::Int:        { prefix = "vec2(float("; suffix = "))"; } break;
            case QMetaType::Float:      { prefix = "vec2("; suffix = ")"; } break;
            case QMetaType::QVector3D:
            case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
            case QMetaType::QTransform: { prefix = ""; suffix = "[0].xy"; } break;
            case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xy"; } break;
            case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).xy"; } break;
            default: break;
            }
        } break;
        case QMetaType::QVector3D: {
            switch(current) {
            case QMetaType::Int:        { prefix = "vec3(float("; suffix = "))"; } break;
            case QMetaType::Float:      { prefix = "vec3("; suffix = ")"; } break;
            case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
            case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
            case QMetaType::QTransform: { prefix = ""; suffix = "[0].xyz"; } break;
            case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xyz"; } break;
            case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).xyz"; } break;
            default: break;
            }
        } break;
        case QMetaType::QVector4D: {
            switch(current) {
            case QMetaType::Int:        { prefix = "vec4(float("; suffix = "))"; } break;
            case QMetaType::Float:      { prefix = "vec4("; suffix = ")"; } break;
            case QMetaType::QVector2D:  { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
            case QMetaType::QVector3D:  { prefix = "vec4("; suffix = ", 1.0)"; } break;
            case QMetaType::QTransform: { prefix = "vec4("; suffix = "[0], 1.0)"; } break;
            case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0]"; } break;
            case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0)"; } break;
            default: break;
            }
        } break;
        default: break;
    }
    return (prefix + value + suffix);
}

QString ShaderNode::localValue(int type, int index, const QString &value, const QString &name) {
    QString s_name = name;
    if(s_name.isEmpty()) {
        s_name = "local" + QString::number(index);
    }

    return QString("\t%1 %2 = %3;\n").arg(typeToString(type), s_name, value);
}

QString ShaderNode::typeToString(int type) {
    switch(type) {
        case QMetaType::Int: return "int"; break;
        case QMetaType::QVector2D: return "vec2"; break;
        case QMetaType::QVector3D: return "vec3"; break;
        case QMetaType::QVector4D: return "vec4"; break;
        case QMetaType::QTransform: return "mat3"; break;
        case QMetaType::QMatrix4x4: return "mat4"; break;
        default: return "float"; break;
    }

    return QString();
}

Variant ShaderNode::fromQVariant(const QVariant &value) {
    switch(value.type()) {
        case QMetaType::Int: {
            return Variant(value.toInt());
        } break;
        case QMetaType::Float: {
            return Variant(value.toFloat());
        } break;
        case QMetaType::QVector2D: {
            QVector2D v = value.value<QVector2D>();
            return Variant(Vector2(v.x(), v.y()));
        } break;
        case QMetaType::QVector3D: {
            QVector3D v = value.value<QVector3D>();
            return Variant(Vector3(v.x(), v.y(), v.z()));
        } break;
        case QMetaType::QVector4D: {
            QVector4D v = value.value<QVector4D>();
            return Variant(Vector4(v.x(), v.y(), v.z(), v.w()));
        } break;
        default: break;
    }

    return Variant();
}

void ShaderNode::switchPreview() {
    Actor *p = m_preview->actor();
//    if(p->isEnabled()) {
//        rect->setRotation(Vector3(0.0f, 0.0f, 90.0f));
//    } else {
//        rect->setRotation(Vector3(0.0f, 0.0f, 0.0f));
//    }
    p->setEnabled(!p->isEnabled());

    ShaderGraph *graph = static_cast<ShaderGraph *>(GraphNode::graph());
    graph->setPreviewVisible(this, p->isEnabled());

    RectTransform *rect = m_nodeWidget->rectTransform();
    Layout *layout = rect->layout();

    Vector2 size = rect->size();

    if(layout) {
        size.y = layout->sizeHint().y;
        layout->invalidate();
    }

    rect->setSize(size);
}

Widget *ShaderNode::widget() {
    Widget *result = GraphNode::widget();

    if(!m_preview) {
        // Add preview if exist
        ShaderGraph *graph = static_cast<ShaderGraph *>(GraphNode::graph());
        if(graph) {
            Texture *preview = graph->preview(this);
            if(preview) {
                Actor *actor = Engine::composeActor(gImage, "Preview", result->actor());
                m_preview = static_cast<Image *>(actor->component(gImage));
                if(m_preview) {
                    m_preview->setTexture(preview);
                    m_preview->setDrawMode(SpriteRender::Simple);

                    RectTransform *r = m_preview->rectTransform();
                    r->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                    r->setSize(Vector2(preview->width(), preview->height()));
                    r->setPivot(Vector2(0.5f, 1.0f));

                    Layout *layout = result->rectTransform()->layout();
                    if(layout) {
                        layout->addTransform(r);
                    }
                }
                actor->setEnabled(false);

                Actor *title = result->actor();
                for(auto it : title->getChildren()) {
                    if(it->name() == "Title") {
                        title = static_cast<Actor *>(it);
                    }
                }

                Actor *icon = Engine::composeActor(gButton, gImage, title);
                m_previewBtn = static_cast<Button *>(icon->component(gButton));

                m_previewBtn->setText("");
                m_previewBtn->icon()->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
                m_previewBtn->icon()->setItem("Arrow");
                m_previewBtn->setIconSize(Vector2(16.0f, 8.0f));

                bool res = Object::connect(m_previewBtn, _SIGNAL(clicked()), m_observer, _SLOT(onPreview()));

                RectTransform *previewRect = m_previewBtn->rectTransform();
                if(previewRect) {
                    previewRect->setSize(Vector2(16.0f));
                    previewRect->setMargin(Vector4(0.0f, 10.0f, 0.0f, 10.0f));
                    previewRect->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
                    previewRect->setPivot(Vector2(1.0f, 0.5f));
                    //previewRect->setRotation(Vector3(0.0f, 0.0f, 90.0f));
                }
            }
        }
    }

    return result;
}
