#ifndef QBSBUILDER_H
#define QBSBUILDER_H

#include <editor/codebuilder.h>

#include <QFileInfo>
#include <QProcess>

class QbsProxy;

class QbsBuilder : public CodeBuilder {
public:
    QbsBuilder();

    void parseLogs(const QString &log);

    void builderInit();

    void onBuildFinished(int exitCode);

    void onApplySettings();

protected:
    bool isNative() const override;

    bool buildProject() override;

    QString builderVersion() override;

    QStringList suffixes() const override { return {"cpp", "h"}; }

    QStringList platforms() const override { return {"desktop", "android"}; }

    QString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    QString getProfile(const QString &platform) const;
    QStringList getArchitectures(const QString &platform) const;

    void setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs);

    void generateProject();

    bool checkProfiles();

    bool isBundle(const QString &platform) const override;

    QString m_artifact;

    QFileInfo m_qbsPath;

    QStringList m_includePath;
    QStringList m_libPath;
    QStringList m_libs;

    QProcess *m_process;
    QbsProxy *m_proxy;

    QStringList m_settings;

    bool m_progress;

};

class QbsProxy : public QObject {
    Q_OBJECT
public:
    void setBuilder(QbsBuilder *builder) {
        m_builder = builder;
    }

public slots:
    void onBuildFinished(int code) {
        m_builder->onBuildFinished(code);
    }

    void readOutput() {
        QProcess *p = dynamic_cast<QProcess *>( QObject::sender() );
        if(p) {
            m_builder->parseLogs(p->readAllStandardOutput());
        }
    }

    void readError() {
        QProcess *p = dynamic_cast<QProcess *>( QObject::sender() );
        if(p) {
            m_builder->parseLogs(p->readAllStandardError());
        }
    }

    void onApplySettings() {
        m_builder->onApplySettings();
    }

private:
    QbsBuilder *m_builder;

};

#endif // QBSBUILDER_H
