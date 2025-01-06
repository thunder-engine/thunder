#ifndef EMSCRIPTENBUILDER_H
#define EMSCRIPTENBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

class QProcess;

class EmscriptenBuilder : public CodeBuilder {
    Q_OBJECT
public:
    EmscriptenBuilder();

protected slots:
    void readOutput();

    void readError();

    void onApplySettings();

    void onBuildFinished(int exitCode);

protected:
    bool isNative() const override { return true; }

    bool isEmpty() const override;

    bool buildProject() override;

    QString builderVersion() override;

    QStringList suffixes() const override { return {"cpp", "h"}; }

    QStringList platforms() const override { return {"webgl"}; }

    QString templatePath() const override { return ":/Templates/Native_Behaviour.cpp"; }

    void generateProject();

    void parseLogs(const QString &log);

    bool isBundle(const QString &platform) const override;

    QString m_artifact;

    QString m_binary;

    QStringList m_includePath;
    QStringList m_libPath;
    QStringList m_libs;

    QProcess *m_process;

    bool m_progress;

};

#endif // EMSCRIPTENBUILDER_H
