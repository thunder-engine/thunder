#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/codebuilder.h>

class QProcess;
class ProjectManager;

class XcodeBuilder : public CodeBuilder {
public:
    XcodeBuilder ();

    bool buildProject ();

    QString builderVersion ();

    QStringList suffixes () const { return {"cpp", "h"}; }

private:
    void generateProject();

private:
    QProcess *m_pProcess;

    QStringList m_Settings;

    ProjectManager *m_pMgr;

    bool m_Progress;
};

#endif // XCODEBUILDER_H
