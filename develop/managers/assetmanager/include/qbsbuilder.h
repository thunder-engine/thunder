#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <converters/converter.h>
#include <converters/builder.h>

#include <QFileInfo>

class QProcess;

class QbsBuilder : public IBuilder {
    Q_OBJECT
public:
    QbsBuilder                      ();

    bool                            buildProject        ();

    QString                         builderVersion      ();

    QStringList                     suffixes            () const { return {"cpp", "h"}; }

public slots:
    void                            builderInit         ();

protected slots:
    void                            readOutput          ();

    void                            readError           ();

    void                            onBuildFinished     (int exitCode);

protected:
    void                            setEnvironment      (const QStringList &incp, const QStringList &libp, const QStringList &libs);

    void                            generateProject     ();

    void                            parseLogs           (const QString &log);

    bool                            checkProfiles       ();

    QString                         m_Artifact;

    QStringList                     m_IncludePath;
    QStringList                     m_LibPath;
    QStringList                     m_Libs;

    QProcess                       *m_pProcess;

    QStringList                     m_Settings;

    ProjectManager                 *m_pMgr;

    bool                            m_Progress;

    QFileInfo                       m_QBSPath;
};

#endif // QBSBUILDER_H
