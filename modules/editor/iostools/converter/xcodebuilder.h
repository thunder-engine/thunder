#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/codebuilder.h>

class QProcess;
class ProjectManager;

class XcodeBuilder : public CodeBuilder {
public:
    XcodeBuilder ();

private:
    bool isNative() const Q_DECL_OVERRIDE;

    bool buildProject () Q_DECL_OVERRIDE;

    QString builderVersion () Q_DECL_OVERRIDE;

    QStringList suffixes () const Q_DECL_OVERRIDE { return {"cpp", "h"}; }

    QStringList platforms() const Q_DECL_OVERRIDE { return {"ios", "tvos"}; }

private:
    QProcess *m_process;

    QStringList m_settings;

    bool m_progress;

};

#endif // XCODEBUILDER_H
