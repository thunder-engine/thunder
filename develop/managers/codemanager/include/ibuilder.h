#ifndef IBUILDER_H
#define IBUILDER_H

#include <QObject>

#include <QMap>

class ProjectManager;

typedef QMap<QString, QString>      StringMap;

class IBuilder : public QObject {
    Q_OBJECT
public:
    IBuilder                        ();

    virtual void                    generateProject     (const QStringList &code) = 0;

    virtual bool                    buildProject        () = 0;

    virtual QString                 builderVersion      () = 0;

    virtual void                    builderInit         () = 0;

    void                            copyTemplate        (const QString &src, const QString &dst, StringMap &values);

    void                            setEnvironment      (const QStringList &incp, const QStringList &libp, const QStringList &libs);

    QString                         project             () const { return m_Project; }

    QString                         artifact            () const { return m_Artifact; }

signals:
    void                            buildFinished       (int exitCode);

protected:
    StringMap                       m_Values;

    QStringList                     m_IncludePath;
    QStringList                     m_LibPath;
    QStringList                     m_Libs;

    QString                         m_Suffix;
    QString                         m_Project;
    QString                         m_Artifact;

    ProjectManager                 *m_pMgr;
};

#endif // IBUILDER_H
