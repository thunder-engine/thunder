#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/codebuilder.h>

class QProcess;

class XcodeBuilder : public CodeBuilder {
public:
    XcodeBuilder ();

private:
    bool isNative() const override;

    bool buildProject () override;

    TString builderVersion () override;

    StringList suffixes () const override { return {"cpp", "h"}; }

    StringList platforms() const override { return {"ios", "tvos"}; }

private:
    StringList m_settings;

    QProcess *m_process;

    bool m_progress;

};

#endif // XCODEBUILDER_H
