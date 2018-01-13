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

public slots:
    void            convert         ();
    void            archive         ();
    void            onCompileDone   (const QString &path);
};

#endif // BUILDER_H
