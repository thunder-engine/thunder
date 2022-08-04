#ifndef UTILS
#define UTILS

#include "function.h"

#define IN  "In"

#define AGB "A>B"
#define BGA "A<B"
#define AEB "A==B"

class Mask : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

    Q_PROPERTY(bool R READ r WRITE setR NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool G READ g WRITE setG NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool B READ b WRITE setB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool A READ a WRITE setA NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Mask() :
        m_r(true),
        m_g(true),
        m_b(true),
        m_a(true) {

        ports.push_back(new NodePort(false, QMetaType::Void, 0, IN));
        ports.push_back(new NodePort(true, QMetaType::Void, 0, ""));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        const AbstractNodeGraph::Link *l = graph->findLink(this, ports.front());
        if(l) {
            ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);

            int32_t type = 0;
            int32_t index = node->build(code, stack, graph, *l, depth, type);
            if(index >= 0) {
                QString mask;
                if(m_r && type > 0) {
                    mask += "r";
                    size = QMetaType::Double;
                }
                if(m_g && type > 1) {
                    mask += "g";
                    if(size == 0) {
                        size = QMetaType::Double;
                    } else {
                        size = QMetaType::QVector2D;
                    }
                }
                if(m_b && type > 2) {
                    mask += "b";
                    if(size == 0) {
                        size = QMetaType::Double;
                    } else if(size == QMetaType::Double) {
                        size = QMetaType::QVector2D;
                    } else {
                        size++;
                    }
                }
                if(m_a && type > 3) {
                    mask += "a";
                    if(size == 0) {
                        size = QMetaType::Double;
                    } else if(size == QMetaType::Double) {
                        size = QMetaType::QVector2D;
                    } else {
                        size++;
                    }
                }

                QString value;
                if(stack.isEmpty()) {
                    value = QString("local%1.%2").arg(QString::number(index), mask);
                } else {
                    value = QString("%1.%2").arg(stack.pop(), mask);
                }

                if(graph->isSingleConnection(link.oport)) {
                    stack.push(value);
                } else {
                    QString type;
                    switch(size) {
                        case QMetaType::QVector2D: type = "\tvec2";  break;
                        case QMetaType::QVector3D: type = "\tvec3";  break;
                        case QMetaType::QVector4D: type = "\tvec4";  break;
                        default: type = "\tfloat"; break;
                    }

                    code.append(QString("%1 local%2 = %3;\n").arg(type, QString::number(depth), value));
                }

            } else {
                return -1;
            }
        } else {
            graph->reportMessage(this, "Missing argument");
            return m_position;
        }
        return ShaderFunction::build(code, stack, graph, link, depth, size);
    }

    bool r() const { return m_r; }
    bool g() const { return m_g; }
    bool b() const { return m_b; }
    bool a() const { return m_a; }

    void setR(bool code) { m_r = code; emit updated();}
    void setG(bool code) { m_g = code; emit updated();}
    void setB(bool code) { m_b = code; emit updated();}
    void setA(bool code) { m_a = code; emit updated();}

private:
    bool m_r;
    bool m_g;
    bool m_b;
    bool m_a;
};

class Fresnel : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Functions")

    Q_PROPERTY(float Power READ power WRITE setPower NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Fresnel() :
        m_power(5.0f) {

        ports.push_back(new NodePort(false, QMetaType::QVector3D, 0, "Normal"));
        ports.push_back(new NodePort(false, QMetaType::QVector3D, 1, "View Dir"));
        ports.push_back(new NodePort(false, QMetaType::Double, 2, "Power"));

        ports.push_back(new NodePort(true,  QMetaType::Double, 0, ""));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        if(m_position == -1) {
            const AbstractNodeGraph::Link *nl = graph->findLink(this, ports.at(0));
            const AbstractNodeGraph::Link *vl = graph->findLink(this, ports.at(1));
            const AbstractNodeGraph::Link *pl = graph->findLink(this, ports.at(2));

            QString normal("_n");
            if(nl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(nl->sender);
                uint32_t index = node->build(code, stack, graph, *nl, depth, type);

                if(stack.isEmpty()) {
                    normal = convert("local" + QString::number(index), type, QMetaType::QVector3D);
                } else {
                    normal = convert(stack.pop(), type, QMetaType::QVector3D);
                }
            }

            QString view("_view");
            if(vl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(vl->sender);
                uint32_t index = node->build(code, stack, graph, *vl, depth, type);

                if(stack.isEmpty()) {
                    view = convert("local" + QString::number(index), type, QMetaType::QVector3D);
                } else {
                    view = convert(stack.pop(), type, QMetaType::QVector3D);
                }
            }

            QString power = QString::number(m_power);
            if(pl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(pl->sender);
                uint32_t index = node->build(code, stack, graph, *pl, depth, type);

                if(stack.isEmpty()) {
                    power = convert("local" + QString::number(index), type, QMetaType::Double);
                } else {
                    power = convert(stack.pop(), type, QMetaType::Double);
                }
            }

            // f0 = 0.04
            code.append(QString("float local%1 = 0.04 + (1.0 - 0.04) * pow(1.0 - dot(%2, -%3), %4);\n").arg(QString::number(depth), normal, view, power));
        }

        return ShaderFunction::build(code, stack, graph, link, depth, size);
    }

    float power() const {
        return m_power;
    }

    void setPower(float power) {
        m_power = power;
        emit updated();
    }

private:
    float m_power;

};

class If : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    Q_INVOKABLE If() {
        ports.push_back(new NodePort(false, QMetaType::Double, 0, a));
        ports.push_back(new NodePort(false, QMetaType::Double, 1, b));
        ports.push_back(new NodePort(false, QMetaType::Void, 2, AGB));
        ports.push_back(new NodePort(false, QMetaType::Void, 3, AEB));
        ports.push_back(new NodePort(false, QMetaType::Void, 4, BGA));

        ports.push_back(new NodePort(true,  QMetaType::Void, 0, ""));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        const AbstractNodeGraph::Link *al  = graph->findLink(this, ports.at(0));
        const AbstractNodeGraph::Link *bl  = graph->findLink(this, ports.at(1));
        const AbstractNodeGraph::Link *agbl= graph->findLink(this, ports.at(2)); // AGB
        const AbstractNodeGraph::Link *aebl= graph->findLink(this, ports.at(3)); // AEB
        const AbstractNodeGraph::Link *bgal= graph->findLink(this, ports.at(4)); // BGA

        if(al && agbl && bgal) {
            ShaderFunction *aNode = static_cast<ShaderFunction *>(al->sender);
            uint32_t aIndex = aNode->build(code, stack, graph, *al, depth, size);
            QString aValue;
            if(stack.isEmpty()) {
                aValue = "local" + QString::number(aIndex);
            } else {
                aValue = stack.pop();
            }

            QString bValue("0.0");
            if(bl) {
                ShaderFunction *bNode = static_cast<ShaderFunction *>(bl->sender);
                uint32_t bIndex = bNode->build(code, stack, graph, *bl, depth, size);

                if(stack.isEmpty()) {
                    bValue = "local" + QString::number(bIndex);
                } else {
                    bValue = stack.pop();
                }
            }

            ShaderFunction *agbNode = static_cast<ShaderFunction *>(agbl->sender);
            uint32_t agbIndex = agbNode->build(code, stack, graph, *agbl, depth, size);
            QString agbValue;
            if(stack.isEmpty()) {
                agbValue = "local" + QString::number(agbIndex);
            } else {
                agbValue = stack.pop();
            }

            QString aebValue;
            if(aebl) {
                ShaderFunction *aebNode = static_cast<ShaderFunction *>(aebl->sender);
                uint32_t aebIndex = aebNode->build(code, stack, graph, *aebl, depth, size);
                if(stack.isEmpty()) {
                    aebValue = "local" + QString::number(aebIndex);
                } else {
                    aebValue = stack.pop();
                }
            }

            ShaderFunction *bgaNode = static_cast<ShaderFunction *>(bgal->sender);
            uint32_t bgaIndex = bgaNode->build(code, stack, graph, *bgal, depth, size);
            QString bgaValue;
            if(stack.isEmpty()) {
                bgaValue = "local" + QString::number(bgaIndex);
            } else {
                bgaValue = stack.pop();
            }

            QString args = QString("(%1 >= %2) ? %3 : %4").arg(aValue, bValue, agbValue, bgaValue);
            if(aebl) {
                args = QString("((abs(%1 - %2) > 0.00001) ? (%3) : %4)").arg(aValue, bValue, args, aebValue);
            }
            switch(size) {
                case QMetaType::QVector2D: code.append("\tvec2"); break;
                case QMetaType::QVector3D: code.append("\tvec3"); break;
                case QMetaType::QVector4D: code.append("\tvec4"); break;
                default: code.append("\tfloat"); break;
            }
            code.append(QString(" local%1 = %2;\n").arg(depth).arg(args));

        } else {
            graph->reportMessage(this, "Missing argument");
            return m_position;
        }

        return ShaderFunction::build(code, stack, graph, link, depth, size);
    }
};
#endif // UTILS

