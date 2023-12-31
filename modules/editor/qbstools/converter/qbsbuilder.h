#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

#include <QFileInfo>

class QProcess;

class QbsBuilder : public CodeBuilder {
    Q_OBJECT
public:
    QbsBuilder();

public slots:
    void builderInit();

protected slots:
    void readOutput();

    void readError();

    void onApplySettings();

    void onBuildFinished(int exitCode);

protected:
    bool isNative() const Q_DECL_OVERRIDE;

    bool buildProject() Q_DECL_OVERRIDE;

    QString builderVersion() Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE { return {"cpp", "h"}; }

    QStringList platforms() const Q_DECL_OVERRIDE { return {"desktop", "android"}; }

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Native_Behaviour.cpp"; }

    QString getProfile(const QString &platform) const;
    QStringList getArchitectures(const QString &platform) const;

    void setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs);

    void generateProject();

    void parseLogs(const QString &log);

    bool checkProfiles();

    bool isEmpty() const Q_DECL_OVERRIDE;

    bool isPackage(const QString &platform) const Q_DECL_OVERRIDE;

    QString m_artifact;

    QString m_javaPath;
    QString m_androidSDKPath;
    QString m_androidNDKPath;

    QStringList m_includePath;
    QStringList m_libPath;
    QStringList m_libs;

    QProcess *m_process;

    QStringList m_settings;

    bool m_progress;

    QFileInfo m_qbsPath;
};

#endif // QBSBUILDER_H
