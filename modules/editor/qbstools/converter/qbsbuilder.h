#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <editor/nativecodebuilder.h>

#include <os/aprocess.h>

class QbsBuilder : public NativeCodeBuilder {
    A_OBJECT(QbsBuilder, NativeCodeBuilder, Core)

public:
    QbsBuilder();

protected:
    void builderInit();

    bool buildProject() override;

    StringList platforms() const override { return {"desktop", "android"}; }

    TString getProfile(const TString &platform) const;
    StringList getArchitectures(const TString &platform) const;

    void generateProject() override;

    bool checkProfiles();

protected:
    TString m_qbsPath;

    StringList m_settings;

};

#endif // QBSBUILDER_H
