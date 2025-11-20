#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/nativecodebuilder.h>

class XcodeBuilder : public NativeCodeBuilder {
public:
    XcodeBuilder();

private:
    bool buildProject() override;

    void generateProject() override;

    StringList platforms() const override { return {"macos", "ios", "tvos"}; }

};

#endif // XCODEBUILDER_H
