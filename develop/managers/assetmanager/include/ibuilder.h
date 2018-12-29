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

    virtual bool                    buildProject        () = 0;

    virtual QString                 builderVersion      () = 0;

    virtual void                    builderInit         () = 0;

    virtual QStringList             suffixes            () const = 0;

    void                            copyTemplate        (const QString &src, const QString &dst, StringMap &values);

    QString                         project             () const { return m_Project; }

    QString                         artifact            () const { return m_Artifact; }

    QStringList                     rescanSources       (const QString &path) const;

signals:
    void                            buildFinished       (int exitCode);

protected:
    StringMap                       m_Values;

    QString                         m_Suffix;
    QString                         m_Project;
    QString                         m_Artifact;

    ProjectManager                 *m_pMgr;
};

#endif // IBUILDER_H
