#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <editor/nativecodebuilder.h>

class QbsBuilder : public NativeCodeBuilder {
    A_OBJECT(QbsBuilder, NativeCodeBuilder, Core)

public:
    QbsBuilder();

protected:
    bool buildProject() override;

    StringList platforms() const override { return {"desktop"}; }

    TString getProfile(const TString &platform) const;
    StringList getArchitectures(const TString &platform) const;

    void generateProject() override;

    bool checkProfiles();

protected:
    TString m_qbsPath;

    StringList m_settings;

};

#endif // QBSBUILDER_H
