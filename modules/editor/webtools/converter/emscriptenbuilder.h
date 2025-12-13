#ifndef EMSCRIPTENBUILDER_H
#define EMSCRIPTENBUILDER_H

#include <editor/assetconverter.h>
#include <editor/nativecodebuilder.h>

class EmscriptenBuilder : public NativeCodeBuilder {
    A_OBJECT(EmscriptenBuilder, NativeCodeBuilder, Core)

public:
    EmscriptenBuilder();

protected: // slots
    void onBuildFinished(int exitCode) override;

protected:
    bool isEmpty() const override;

    bool buildProject() override;

    StringList platforms() const override { return {"webgl"}; }

    PackagingMode packagingMode() const override { return None; }

protected:
    TString m_emPath;

    TString m_binary;

};

#endif // EMSCRIPTENBUILDER_H
