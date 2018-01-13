#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include "ibuilder.h"

class QbsBuilder : public IBuilder {
    Q_OBJECT
public:
    QbsBuilder                      ();

    virtual void                    generateProject     (const QStringList &code);

    virtual bool                    buildProject        ();

protected slots:
    void                            readOutput          ();

    void                            readError           ();

protected:
    QString                         formatList          (const QStringList &list);

    void                            parseLogs           (const QString &log);

};

#endif // QBSBUILDER_H
