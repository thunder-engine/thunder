#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/codebuilder.h>

class QProcess;
class ProjectManager;

class XcodeBuilder : public CodeBuilder {
public:
    XcodeBuilder ();

private:
    bool buildProject () Q_DECL_OVERRIDE;

    QString builderVersion () Q_DECL_OVERRIDE;

    QStringList suffixes () const Q_DECL_OVERRIDE { return {"cpp", "h"}; }

    QStringList platforms() const Q_DECL_OVERRIDE { return {"ios", "tvos"}; }

private:
    QProcess *m_pProcess;

    QStringList m_Settings;

    bool m_Progress;
};

#endif // XCODEBUILDER_H
