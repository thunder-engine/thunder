#ifndef XCODEBUILDER_H
#define XCODEBUILDER_H

#include <editor/codebuilder.h>

#include <QProcess>

class XcodeProxy;

class XcodeBuilder : public CodeBuilder {
public:
    XcodeBuilder ();

private:
    bool isNative() const override;

    bool buildProject () override;

    TString builderVersion () override;

    StringList suffixes () const override { return {"cpp", "h"}; }

    StringList platforms() const override { return {"ios", "tvos"}; }

private:
    StringList m_settings;

    XcodeProxy *m_proxy;

    QProcess *m_process;

    bool m_progress;

};

class XcodeProxy : public QObject {
    Q_OBJECT
public:
    void setBuilder(XcodeBuilder *builder) {
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
    XcodeBuilder *m_builder;

};

#endif // XCODEBUILDER_H
