#ifndef BUILDER_H
#define BUILDER_H

#include <QDirIterator>
#include <QDebug>

#include <quazip.h>
#include <quazipfile.h>

class Builder : public QObject {
    Q_OBJECT
public:
    Builder         ();

signals:
    void            packDone        ();
    void            moveDone        (const QString &target);

public slots:
    void            package         (const QString &target);
    void            onCompileDone   (const QString &path);
};

#endif // BUILDER_H
