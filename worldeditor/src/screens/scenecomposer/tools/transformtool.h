#ifndef TRANSFORMTOOL_H
#define TRANSFORMTOOL_H

#include "selecttool.h"

class TransformTool : public SelectTool {
public:
    explicit TransformTool(ObjectController *controller);

    void update(bool center, bool local, bool snap) override;

protected:
    TString icon() const override;
    TString name() const override;

    TString component() const override;

};

#endif // TRANSFORMTOOL_H
