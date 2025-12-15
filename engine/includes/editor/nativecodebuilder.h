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
    enum RHI {
        Invalid,
        OpenGL,
        Vulkan,
        Metal
    };

    enum PackagingMode {
        None,
        Before,
        After
    };

public:
    NativeCodeBuilder();

    virtual PackagingMode packagingMode() const { return After; }
    virtual bool isEmbedded() const { return false; }
    virtual RHI defaultRhi() const { return OpenGL; }

    bool buildProject() override;

protected: // slots
    void onReadOutput();
    void onReadError();

    virtual void onBuildFinished(int exitCode);

protected:
    TString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    StringList suffixes() const override { return {"cpp", "h"}; }

    void parseLogs(const TString &log);

    virtual void generateProject();

    void generateLoader(const TString &dst, const StringList &modules);

    TString formatList(const StringList &list, const TString &pref, const TString &suff, const TString &sep) const;

protected:
    StringList m_incPath;
    StringList m_libPath;
    StringList m_libs;

    TString m_incPref;
    TString m_incSuff;
    TString m_incSep;

    TString m_libPref;
    TString m_libSuff;
    TString m_libSep;

    TString m_libsPref;
    TString m_libsSuff;
    TString m_libsSep;

    TString m_filePref;
    TString m_fileSuff;
    TString m_fileSep;

    Process m_process;

    TString m_artifact;

};

#endif // NATIVECODEBUILDER_H
