#ifndef ANDROIDBUILDER_H
#define ANDROIDBUILDER_H

#include <editor/nativecodebuilder.h>

class AndroidBuilder : public NativeCodeBuilder {
    A_OBJECT(AndroidBuilder, NativeCodeBuilder, Core)

public:
    AndroidBuilder();

protected:
    bool buildProject() override;

    bool compileNative(const TString &tools, const TString &arch);

    bool compileResources(const TString &tools);
    bool linkResources(const TString &tools, const TString &sdk);

    bool makeKeystore(const TString &java);
    bool signPackage(const TString &java, const TString &tools);

    bool package();

    StringList platforms() const override { return {"android"}; }

    void generateProject() override;

    PackagingMode packagingMode() const override { return None; }
    bool isEmbedded() const override { return true; }

private:
    TString m_projectPath;
    TString m_keystorePath;

};

#endif // ANDROIDBUILDER_H
