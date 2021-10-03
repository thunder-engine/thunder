#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

#include <QFileInfo>

class QProcess;
class ProjectManager;

class QbsBuilder : public CodeBuilder {
    Q_OBJECT
public:
    QbsBuilder();

public slots:
    void builderInit();

protected slots:
    void readOutput();

    void readError();

    void onBuildFinished(int exitCode);

protected:
    bool buildProject() Q_DECL_OVERRIDE;

    QString builderVersion() Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE { return {"cpp", "h"}; }

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Native_Behaviour.cpp"; }

    void setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs);

    void generateProject();

    void parseLogs(const QString &log);

    bool checkProfiles();

    bool isEmpty() const Q_DECL_OVERRIDE;

    QString m_Artifact;

    QStringList m_IncludePath;
    QStringList m_LibPath;
    QStringList m_Libs;

    QProcess *m_pProcess;

    QStringList m_Settings;

    ProjectManager *m_pMgr;

    bool m_Progress;

    QFileInfo m_QBSPath;
};

#endif // QBSBUILDER_H
