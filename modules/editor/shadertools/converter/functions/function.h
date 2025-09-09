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

namespace {
    const char *a("A");
    const char *b("B");
    const char *d("D");
    const char *r("R");
    const char *g("G");
    const char *x("X");
    const char *y("Y");
    const char *z("Z");
    const char *w("W");

    const char *gImage("Image");
    const char *gButton("Button");
}

class ShaderNode : public GraphNode {
    A_OBJECT(ShaderNode, GraphNode, Graph)

    A_METHODS(
        A_SLOT(ShaderNode::switchPreview)
    )
    A_NOPROPERTIES()
    A_NOENUMS()

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

    virtual QString defaultValue(const TString &key, uint32_t &type) const {
        A_UNUSED(key);
        type = MetaType::INVALID;
        return QString();
    }

    virtual int getOutType(int inType, const AbstractNodeGraph::Link *l) {
        A_UNUSED(l);

        if(m_type == 0) {
            m_type = inType;
        }
        return m_type;
    }

    static QString convert(const QString &value, uint32_t current, uint32_t target, uint8_t component = 0);

    static QString localValue(int type, int index, const QString &value, const QString &name = QString());

    static QString typeToString(int type);

    void switchPreview();

    Widget *widget() override;

    void onPreview();

protected:
    friend class ShaderGraph;

    Image *m_preview;
    Button *m_previewBtn;

    std::list<std::pair<TString, uint32_t>> m_inputs;
    std::list<std::pair<TString, uint32_t>> m_outputs;

    QString m_expression;

    int32_t m_position;
    int32_t m_type;

    static std::map<uint32_t, Vector4> m_portColors;

};

#endif // FUNCTION_H
