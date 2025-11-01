#ifndef NATIVECODEBUILDER_H
#define NATIVECODEBUILDER_H

#include "codebuilder.h"

#include <os/aprocess.h>

class ENGINE_EXPORT NativeCodeBuilder : public CodeBuilder {
    A_OBJECT(NativeCodeBuilder, CodeBuilder, Core)

    A_METHODS(
        A_SLOT(NativeCodeBuilder::onReadOutput),
        A_SLOT(NativeCodeBuilder::onReadError),
        A_SLOT(NativeCodeBuilder::onBuildFinished)
    )

public:
    NativeCodeBuilder();

protected: // slots
    void onReadOutput();
    void onReadError();

    virtual void onBuildFinished(int exitCode);

    bool buildProject() override;

protected:
    TString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    StringList suffixes() const override { return {"cpp", "h"}; }

    void parseLogs(const TString &log);

    virtual void generateProject();

    void generateLoader(const TString &dst, const StringList &modules);

    TString formatList(const StringList &list) const;

    bool isBundle(const TString &platform) const override;

protected:
    StringList m_includePath;
    StringList m_libPath;
    StringList m_libs;

    Process m_process;

    TString m_artifact;

};

#endif // NATIVECODEBUILDER_H
