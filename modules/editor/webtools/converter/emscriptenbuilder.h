#ifndef EMSCRIPTENBUILDER_H
#define EMSCRIPTENBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

#include <QProcess>

class EmscriptenProxy;

class EmscriptenBuilder : public CodeBuilder {
public:
    EmscriptenBuilder();

    void parseLogs(const QString &log);

    void readOutput();

    void readError();

    void onApplySettings();

    void onBuildFinished(int exitCode);

protected:
    bool isNative() const override { return true; }

    bool isEmpty() const override;

    bool buildProject() override;

    StringList suffixes() const override { return {"cpp", "h"}; }

    StringList platforms() const override { return {"webgl"}; }

    TString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    void generateProject();

    bool isBundle(const TString &platform) const override;

    TString m_artifact;

    TString m_binary;

    StringList m_includePath;
    StringList m_libPath;
    StringList m_libs;

    QProcess *m_process;
    EmscriptenProxy *m_proxy;

    bool m_progress;

};

class EmscriptenProxy : public QObject {
    Q_OBJECT
public:
    void setBuilder(EmscriptenBuilder *builder) {
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

private:
    EmscriptenBuilder *m_builder;

};

#endif // EMSCRIPTENBUILDER_H
