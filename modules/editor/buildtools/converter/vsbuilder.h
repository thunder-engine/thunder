#ifndef VSBUILDER_H
#define VSBUILDER_H

#include <editor/nativecodebuilder.h>
#include <editor/projectsettings.h>

class VsBuilder : public NativeCodeBuilder {
    A_OBJECT(VsBuilder, NativeCodeBuilder, Core)

public:
    VsBuilder();

protected:
    bool buildProject() override;

    StringList platforms() const override { return {"windows"}; }

    void generateProject() override;

    StringList platformLibraries() const override { return Editor::project()->targetPath().isEmpty() ? StringList() : StringList({"glfw", "glad"}); }

protected:
    TString m_vsPath;

};

#endif // VSBUILDER_H
