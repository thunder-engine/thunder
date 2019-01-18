#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include "converters/builder.h"

class QProcess;

class QbsBuilder : public IBuilder {
    Q_OBJECT
public:
    QbsBuilder                      ();

    bool                            buildProject        ();

    QString                         builderVersion      ();

    QStringList                     suffixes            () const { return {"cpp", "h"}; }

    void                            setEnvironment      (const QStringList &incp, const QStringList &libp, const QStringList &libs);

protected slots:
    void                            readOutput          ();

    void                            readError           ();

protected:
    void                            builderInit         ();

    void                            generateProject     ();

    QString                         formatList          (const QStringList &list);

    void                            parseLogs           (const QString &log);

    bool                            checkProfile        (const QString &profile);

    QStringList                     m_IncludePath;
    QStringList                     m_LibPath;
    QStringList                     m_Libs;

    QProcess                       *m_pProcess;

    QStringList                     m_Settings;

    QStringList                     m_Profiles;

    ProjectManager                 *m_pMgr;
};

#endif // QBSBUILDER_H
