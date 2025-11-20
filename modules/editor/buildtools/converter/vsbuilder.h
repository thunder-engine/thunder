#ifndef VSBUILDER_H
#define VSBUILDER_H

#include <editor/nativecodebuilder.h>

class VsBuilder : public NativeCodeBuilder {
    A_OBJECT(VsBuilder, NativeCodeBuilder, Core)

public:
    VsBuilder();

protected:
    bool buildProject() override;

    StringList platforms() const override { return {"windows"}; }

    void generateProject() override;

    bool isBundle(const TString &) const override { return false; }

protected:
    TString m_vsPath;

};

#endif // VSBUILDER_H
