#include "function.h"

ShaderNode::ShaderNode() :
        m_preview(nullptr),
        m_previewBtn(nullptr) {

    reset();
}

ShaderNode::~ShaderNode() {
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

int32_t ShaderNode::build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) {
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

int32_t ShaderNode::compile(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) {
    if(m_position == -1) {
        std::vector<TString> args = getArguments(code, stack, depth, type);

        if(args.size() == m_inputs.size()) {
            TString expr = makeExpression(args);
            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(type, depth, expr));
            }
        } else {
            reportMessage("Missing argument");
            return m_position;
        }
    } else {
        type = m_type;
    }

    return ShaderNode::build(code, stack, link, depth, type);
}

std::vector<TString> ShaderNode::getArguments(TString &code, std::stack<TString> &stack, int32_t &depth, int32_t &type) {
    std::vector<TString> result;

    for(const NodePort &it : m_ports) {
        if(it.m_out == true) {
            continue;
        }

        uint32_t defaultType = 0;
        TString value = defaultValue(it.m_name, defaultType);

        const GraphLink *l = m_graph->findLink(this, &it);
        if(l) {
            ShaderNode *node = static_cast<ShaderNode *>(l->sender);

            int32_t l_type = 0;
            int32_t index = node->build(code, stack, *l, depth, l_type);
            if(index >= 0) {
                type = getOutType(l_type, l);

                int32_t target = l->iport->m_type == MetaType::INVALID ? type : l->iport->m_type;

                if(stack.empty()) {
                    value = convert(TString("local%1").arg(TString::number(index)), l_type, target);
                } else {
                    value = convert(stack.top(), l_type, target);
                    stack.pop();
                }
            }
        }

        if(!value.isEmpty()) {
            result.push_back(value);
        }
    }

    return result;
}

TString ShaderNode::convert(const TString &value, uint32_t current, uint32_t target, uint8_t component) {
    TString prefix;
    TString suffix;

    const char *names[] = {".x", ".y", ".z", ".w"};

    switch(target) {
        case MetaType::INTEGER: {
            switch(current) {
            case MetaType::FLOAT: { prefix = "int("; suffix = ")"; } break;
            case MetaType::VECTOR2:
            case MetaType::VECTOR3:
            case MetaType::VECTOR4: { prefix = "int("; suffix = TString(names[component]) + ")"; } break;
            case MetaType::MATRIX3:
            case MetaType::MATRIX4: { prefix = "int("; suffix = TString("[0]") + TString(names[component]) + ")"; } break;
            case MetaType::STRING:  { prefix = "int(texture("; suffix = ", _uv0).x)"; } break;
            default: break;
            }
        }
        case MetaType::FLOAT: {
            switch(current) {
            case MetaType::INTEGER: { prefix = "float("; suffix = ")"; } break;
            case MetaType::VECTOR2:
            case MetaType::VECTOR3:
            case MetaType::VECTOR4: { prefix = ""; suffix = names[component]; } break;
            case MetaType::MATRIX3:
            case MetaType::MATRIX4: { prefix = ""; suffix = TString("[0]") + names[component]; } break;
            case MetaType::STRING:  { prefix = "texture("; suffix = ", _uv0).x"; } break;
            default: break;
            }
        } break;
        case MetaType::VECTOR2: {
            switch(current) {
            case MetaType::INTEGER: { prefix = "vec2(float("; suffix = "))"; } break;
            case MetaType::FLOAT:   { prefix = "vec2("; suffix = ")"; } break;
            case MetaType::VECTOR3:
            case MetaType::VECTOR4: { prefix = ""; suffix = ".xy"; } break;
            case MetaType::MATRIX3: { prefix = ""; suffix = "[0].xy"; } break;
            case MetaType::MATRIX4: { prefix = ""; suffix = "[0].xy"; } break;
            case MetaType::STRING:  { prefix = "texture("; suffix = ", _uv0).xy"; } break;
            default: break;
            }
        } break;
        case MetaType::VECTOR3: {
            switch(current) {
            case MetaType::INTEGER: { prefix = "vec3(float("; suffix = "))"; } break;
            case MetaType::FLOAT:   { prefix = "vec3("; suffix = ")"; } break;
            case MetaType::VECTOR2: { prefix = "vec3("; suffix = ", 0.0)"; } break;
            case MetaType::VECTOR4: { prefix = ""; suffix = ".xyz"; } break;
            case MetaType::MATRIX3: { prefix = ""; suffix = "[0].xyz"; } break;
            case MetaType::MATRIX4: { prefix = ""; suffix = "[0].xyz"; } break;
            case MetaType::STRING:  { prefix = "texture("; suffix = ", _uv0).xyz"; } break;
            default: break;
            }
        } break;
        case MetaType::VECTOR4: {
            switch(current) {
            case MetaType::INTEGER: { prefix = "vec4(float("; suffix = "))"; } break;
            case MetaType::FLOAT: { prefix = "vec4("; suffix = ")"; } break;
            case MetaType::VECTOR2: { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
            case MetaType::VECTOR3: { prefix = "vec4("; suffix = ", 1.0)"; } break;
            case MetaType::MATRIX3: { prefix = "vec4("; suffix = "[0], 1.0)"; } break;
            case MetaType::MATRIX4: { prefix = ""; suffix = "[0]"; } break;
            case MetaType::STRING:  { prefix = "texture("; suffix = ", _uv0)"; } break;
            default: break;
            }
        } break;
        default: break;
    }
    return (prefix + value + suffix);
}

TString ShaderNode::localValue(int type, int index, const TString &value, const TString &name) {
    TString s_name = name;
    if(s_name.isEmpty()) {
        s_name = TString("local") + TString::number(index);
    }

    return TString("\t%1 %2 = %3;\n").arg(typeToString(type), s_name, value);
}

TString ShaderNode::typeToString(int type) {
    switch(type) {
        case MetaType::INTEGER: return "int"; break;
        case MetaType::VECTOR2: return "vec2"; break;
        case MetaType::VECTOR3: return "vec3"; break;
        case MetaType::VECTOR4: return "vec4"; break;
        case MetaType::MATRIX3: return "mat3"; break;
        case MetaType::MATRIX4: return "mat4"; break;
        default: return "float"; break;
    }

    return TString();
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

    m_nodeWidget->raise();
    RectTransform *rect = m_nodeWidget->rectTransform();
    Layout *layout = rect->layout();

    layout->invalidate();
}

Widget *ShaderNode::widget() {
    Widget *result = GraphNode::widget();

    if(!m_preview) {
        // Add preview if exist
        ShaderGraph *graph = static_cast<ShaderGraph *>(GraphNode::graph());
        if(graph) {
            Texture *preview = graph->preview(this);
            if(preview) {
                Actor *actor = Engine::composeActor<Image>("Preview", result->actor());
                m_preview = actor->getComponent<Image>();
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
                    if(it->name() == "Header") {
                        title = static_cast<Actor *>(it);
                    }
                }

                Actor *icon = Engine::composeActor<Button>("PreviewBtn", title);
                m_previewBtn = icon->getComponent<Button>();

                m_previewBtn->setText("");
                m_previewBtn->icon()->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
                m_previewBtn->setIconSize(Vector2(16.0f, 8.0f));

                Object::connect(m_previewBtn, _SIGNAL(clicked()), this, _SLOT(switchPreview()));

                RectTransform *previewRect = m_previewBtn->rectTransform();
                if(previewRect) {
                    previewRect->setSize(Vector2(16.0f));
                    previewRect->setMargin(Vector4(0.0f, 10.0f, 0.0f, 10.0f));
                    previewRect->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
                    previewRect->setPivot(Vector2(1.0f, 0.5f));
                    previewRect->setPosition(Vector3(-16.0f, 0.0f, 0.0f));
                }
            }
        }
    }

    return result;
}
