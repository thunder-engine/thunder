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

    QString                         builderToolchains   ();

protected slots:
    void                            readOutput          ();

    void                            readError           ();

protected:
    QString                         formatList          (const QStringList &list);

    void                            parseLogs           (const QString &log);

    QProcess                       *m_pProcess;
};

#endif // QBSBUILDER_H
