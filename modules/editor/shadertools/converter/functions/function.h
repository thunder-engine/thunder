#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shadergraph.h"

#include <editor/assetconverter.h>
#include <editor/graph/graphnode.h>

#include <components/actor.h>
#include <components/frame.h>
#include <components/label.h>
#include <components/image.h>
#include <components/button.h>
#include <components/recttransform.h>
#include <components/layout.h>

#include <components/spriterender.h>

#include <QStack>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QDynamicPropertyChangeEvent>

namespace {
    static const char *a("A");
    static const char *b("B");
    static const char *d("D");
    static const char *r("R");
    static const char *g("G");
    static const char *x("X");
    static const char *y("Y");
    static const char *z("Z");
    static const char *w("W");

    const char *gImage("Image");
    const char *gButton("Button");
}

class FunctionObserver;

class ShaderNode : public GraphNode {
    Q_OBJECT

public:
    ShaderNode();
    ~ShaderNode();

    void reset() {
        m_position = -1;
        m_type = 0;
    }

    Vector4 color() const override {
        return Vector4(0.513f, 0.192f, 0.290f, 1.0f);
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    virtual void createParams();

    virtual int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type);

    int32_t compile(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type);

    virtual QString makeExpression(const QStringList &args) const {
        return QString("%1(%2)").arg(m_expression, args.join(", "));
    }

    QStringList getArguments(QString &code, QStack<QString> &stack, int32_t &depth, int32_t &type);

    virtual QString defaultValue(const std::string &key, uint32_t &type) const {
        Q_UNUSED(key);
        type = QMetaType::Void;
        return QString();
    }

    virtual int getOutType(int inType, const AbstractNodeGraph::Link *l) {
        Q_UNUSED(l);

        if(m_type == 0) {
            m_type = inType;
        }
        return m_type;
    }

    static QString convert(const QString &value, uint32_t current, uint32_t target, uint8_t component = 0);

    static QString localValue(int type, int index, const QString &value, const QString &name = QString());

    static QString typeToString(int type);

    static Variant fromQVariant(const QVariant &value);

    void switchPreview();

    Widget *widget() override;

protected:
    friend class ShaderGraph;

    FunctionObserver *m_observer;

    Image *m_preview;
    Button *m_previewBtn;

    std::list<std::pair<std::string, uint32_t>> m_inputs;
    std::list<std::pair<std::string, uint32_t>> m_outputs;

    QString m_expression;

    int32_t m_position;
    int32_t m_type;

    static std::map<uint32_t, Vector4> m_portColors;

};



#endif // FUNCTION_H
