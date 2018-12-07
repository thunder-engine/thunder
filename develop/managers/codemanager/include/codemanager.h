#ifndef CODEMANAGER_H
#define CODEMANAGER_H

#include <QObject>

class IBuilder;
class ProjectManager;

class CodeManager : public QObject {
    Q_OBJECT
public:
    static CodeManager             *instance                    ();

    static void                     destroy                     ();

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

private:
    CodeManager                     () :
            m_Outdated(false),
            m_pBuilder(nullptr),
            m_pProject(nullptr) {

    }
    ~CodeManager                    () {}

    static CodeManager             *m_pInstance;

    QStringList                     rescanSources               (const QString &path);

protected:
    bool                            m_Outdated;

    QStringList                     m_Suffixes;

    IBuilder                       *m_pBuilder;

    ProjectManager                 *m_pProject;
};

#endif // CODEMANAGER_H
