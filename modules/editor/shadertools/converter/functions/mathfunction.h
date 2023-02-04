#ifndef MATHFUNCTIONS
#define MATHFUNCTIONS

#include "function.h"

class Mod : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    Q_INVOKABLE Mod() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("mod", code, stack, link, depth, type);
    }
};

#endif // MATHFUNCTIONS
