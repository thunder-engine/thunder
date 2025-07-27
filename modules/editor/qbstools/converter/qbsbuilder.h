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

    StringList suffixes() const override { return {"cpp", "h"}; }

    StringList platforms() const override { return {"desktop", "android"}; }

    TString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    TString getProfile(const TString &platform) const;
    StringList getArchitectures(const TString &platform) const;

    void setEnvironment(const StringList &incp, const StringList &libp, const StringList &libs);

    void generateProject();

    bool checkProfiles();

    bool isBundle(const TString &platform) const override;

    TString m_artifact;

    QFileInfo m_qbsPath;

    StringList m_includePath;
    StringList m_libPath;
    StringList m_libs;

    StringList m_settings;

    QbsProxy *m_proxy;
    QProcess *m_process;

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
