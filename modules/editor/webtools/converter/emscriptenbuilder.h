#ifndef EMSCRIPTENBUILDER_H
#define EMSCRIPTENBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

#include <os/aprocess.h>

class EmscriptenBuilder : public CodeBuilder {
    A_OBJECT(EmscriptenBuilder, CodeBuilder, Core)

    A_METHODS(
        A_SLOT(EmscriptenBuilder::onApplySettings),
        A_SLOT(EmscriptenBuilder::readOutput),
        A_SLOT(EmscriptenBuilder::readError),
        A_SLOT(EmscriptenBuilder::onBuildFinished)
    )

public:
    EmscriptenBuilder();

protected: // slots
    void readOutput();

    void readError();

    void onApplySettings();

    void onBuildFinished(int exitCode);

protected:
    void parseLogs(const TString &log);

    bool isNative() const override { return true; }

    bool isEmpty() const override;

    bool buildProject() override;

    StringList suffixes() const override { return {"cpp", "h"}; }

    StringList platforms() const override { return {"webgl"}; }

    TString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    void generateProject();

    bool isBundle(const TString &platform) const override;

    TString m_artifact;

    TString m_sdk;
    TString m_binary;

    StringList m_includePath;
    StringList m_libPath;
    StringList m_libs;

    Process m_process;

};

#endif // EMSCRIPTENBUILDER_H
