#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include "ibuilder.h"

class QProcess;

class QbsBuilder : public IBuilder {
    Q_OBJECT
public:
    QbsBuilder                      ();

    void                            generateProject     (const QStringList &code);

    bool                            buildProject        ();

    QString                         builderVersion      ();

    void                            builderInit         ();

protected slots:
    void                            readOutput          ();

    void                            readError           ();

protected:
    QString                         formatList          (const QStringList &list);

    void                            parseLogs           (const QString &log);

    bool                            checkProfile        (const QString &profile);

    QProcess                       *m_pProcess;

    QStringList                     m_Settings;

    QStringList                     m_Profiles;
};

#endif // QBSBUILDER_H
