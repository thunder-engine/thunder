#ifndef CODEMANAGER_H
#define CODEMANAGER_H

#include <QObject>

#include <patterns/asingleton.h>

class IBuilder;
class ProjectManager;

class CodeManager : public QObject, public ASingleton<CodeManager> {
    Q_OBJECT
public:
    void                            init                        ();

    void                            setOutdated                 ();

signals:
    void                            buildSucess                 (const QString &path);

    void                            buildFailed                 ();

public slots:
    void                            buildProject                ();
    void                            rebuildProject              ();

protected slots:
    void                            onBuildFinished             (int exitCode);

protected:
    CodeManager                     () {}
    ~CodeManager                    () {}

    QStringList                     rescanSources               (const QString &path);

protected:
    friend class ASingleton<CodeManager>;

    bool                            m_Outdated;

    QStringList                     m_Suffixes;

    IBuilder                       *m_pBuilder;

    ProjectManager                 *m_pProject;
};

#endif // CODEMANAGER_H
