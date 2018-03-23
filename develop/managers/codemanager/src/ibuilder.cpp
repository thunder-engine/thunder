#include "ibuilder.h"

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>

#include <projectmanager.h>
#include <config.h>

IBuilder::IBuilder() {
    m_pMgr      = ProjectManager::instance();

    m_Suffix    = gShared;
    if(!m_pMgr->targetPath().isEmpty()) {
        m_Suffix= gApplication;
    }

    m_Project   = m_pMgr->generatedPath() + "/";
    m_Artifact  = m_Project + m_pMgr->projectName() + m_Suffix;

    const QMetaObject *meta = m_pMgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        m_Values[QString("${%1}").arg(property.name())]   = property.read(m_pMgr).toString();
    }
}

void IBuilder::copyTemplate(const QString &src, const QString &dst, StringMap &values) {
    QFile file(src);
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data(file.readAll());
        file.close();

        QMapIterator<QString, QString> it(values);
        while(it.hasNext()) {
            it.next();
            data.replace(it.key(), qPrintable(it.value()));
        }
        QFile gen(dst);
        if(gen.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
            gen.write(data);
            gen.close();
        }
    }
}

void IBuilder::setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs) {
    m_IncludePath   = incp;
    m_LibPath       = libp;
    m_Libs          = libs;
}
