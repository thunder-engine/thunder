#ifndef BUILDER_H
#define BUILDER_H

#include <QDirIterator>
#include <QStack>

#include <quazip.h>
#include <quazipfile.h>

class Builder : public QObject {
    Q_OBJECT
public:
    Builder         ();

    void            setPlatform         (const QString &platform);
signals:
    void            packDone            ();
    void            moveDone            (const QString &target);

public slots:
    void            package             (const QString &target);
    void            onImportFinished    ();

private:
    QStack<QString> m_Stack;
};

#endif // BUILDER_H
