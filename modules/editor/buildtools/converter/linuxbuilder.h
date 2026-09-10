#ifndef LINUXBUILDER_H
#define LINUXBUILDER_H

#include <editor/assetconverter.h>
#include <editor/nativecodebuilder.h>

class LinuxBuilder : public NativeCodeBuilder {
    A_OBJECT(LinuxBuilder, NativeCodeBuilder, Core)

public:
    LinuxBuilder();

protected:
    bool isEmpty() const override;

    bool buildProject() override;

    StringList platforms() const override { return {"linux"}; }

    StringList platformLibraries() const override;
};

#endif // LINUXBUILDER_H
