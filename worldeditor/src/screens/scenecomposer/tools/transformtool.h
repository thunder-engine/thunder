#ifndef TRANSFORMTOOL_H
#define TRANSFORMTOOL_H

#include "selecttool.h"

class TransformTool : public SelectTool {
public:
    explicit TransformTool(ObjectController *controller);

    void update(bool center, bool local, bool snap) override;

protected:
    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

};

#endif // TRANSFORMTOOL_H
