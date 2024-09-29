#ifndef EMSCRIPTENBUILDER_H
#define EMSCRIPTENBUILDER_H

#include <editor/assetconverter.h>
#include <editor/codebuilder.h>

#include <QFileInfo>

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
    bool isNative() const Q_DECL_OVERRIDE { return true; }

    bool buildProject() Q_DECL_OVERRIDE;

    QString builderVersion() Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE { return {"cpp", "h"}; }

    QStringList platforms() const Q_DECL_OVERRIDE { return {"webgl"}; }

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Native_Behaviour.cpp"; }

    void generateProject();

    void parseLogs(const QString &log);

    bool isEmpty() const Q_DECL_OVERRIDE;

    bool isPackage(const QString &platform) const Q_DECL_OVERRIDE;

    QString m_artifact;

    QString m_binary;

    QStringList m_includePath;
    QStringList m_libPath;
    QStringList m_libs;

    QProcess *m_process;

    bool m_progress;

};

#endif // EMSCRIPTENBUILDER_H
