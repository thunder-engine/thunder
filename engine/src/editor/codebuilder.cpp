#include "editor/codebuilder.h"

#include "resources/text.h"

#include "editor/assetmanager.h"

#include <QFile>

BuilderSettings::BuilderSettings(CodeBuilder *builder) :
        m_builder(builder) {

}

StringList BuilderSettings::typeNames() const {
    return { MetaType::name<Text>() };
}

CodeBuilder *BuilderSettings::builder() const {
    return m_builder;
}

bool BuilderSettings::isCode() const {
    return true;
}

CodeBuilder::CodeBuilder() :
        m_outdated(false) {

}

void CodeBuilder::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/code.svg");
    }
}

AssetConverter::ReturnCode CodeBuilder::convertFile(AssetConverterSettings *) {
    makeOutdated();
    return Skipped;
}

void CodeBuilder::buildSuccessful(bool flag) {
    AssetManager::instance()->onBuildSuccessful(flag, this);
}

AssetConverterSettings *CodeBuilder::createSettings() {
    return new BuilderSettings(this);
}

void CodeBuilder::renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName) {
    File file(settings->source());
    if(file.open(File::ReadOnly)) {
        TString data(file.readAll());
        file.close();

        static const StringList templates = {
            "class %1",
            "%1()",
            "(%1"
        };

        for(auto &it : templates) {
            data.replace(it.arg(oldName), it.arg(newName));
        }

        if(file.open(File::WriteOnly)) {
            file.write(data);
            file.close();
        }
    }
}

void CodeBuilder::updateTemplate(const TString &src, const TString &dst) {
    QFile file(dst.data());
    if(!file.exists()) {
        file.setFileName(src.data());
    }

    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QString data = file.readLine();

        TString out;

        int begin = -1;
        int row = 0;
        while(!data.isNull()) {
            int index = -1;
            if(begin > -1) {
                index = data.indexOf(QByteArray("//-"));
                if(index != -1) {
                    begin = -1;
                    out += data.toStdString();
                }
            } else {
                auto it = m_values.begin();
                while(it != m_values.end()) {
                    if(it->first.at(0) == '$') {
                        data.replace(it->first.data(), it->second.data());
                    }
                    it++;
                }

                out += data.toStdString();
            }

            index = data.indexOf(QByteArray("//+"));
            if(index != -1) {
                begin = row;

                TString key = data.mid(index + 3).trimmed().toStdString();
                TString value = m_values[TString("$") + key];
                if(!value.isEmpty()) {
                    out += value;
                }
            }

            data = file.readLine();
            row++;
        }
        file.close();

        File::mkPath(Url(dst).dir());

        File outFile(dst);
        if(outFile.open(File::WriteOnly)) {
            outFile.write(out);
            outFile.close();
        }
    }
}

void CodeBuilder::copyTempalte(const TString &src, const TString &dst) {
    if(!File::exists(dst)) {
        File::mkPath(Url(dst).dir());

        QFile::copy(src.data(), dst.data());
    }
}

const TString CodeBuilder::persistentAsset() const {
    return "";
}

const TString CodeBuilder::persistentUUID() const {
    return "";
}

StringList CodeBuilder::platforms() const {
    return StringList();
}

TString CodeBuilder::project() const {
    return m_project;
}

StringList CodeBuilder::sources() const {
    StringList list;
    for(auto &it : m_sources) {
        list.push_back(it);
    }
    return list;
}

void CodeBuilder::rescanSources(const TString &path) {
    m_sources.clear();

    for(auto &filePath : File::list(path)) {
        TString suff = Url(filePath).completeSuffix().toLower();
        for(auto &it : suffixes()) {
            if(it == suff) {
                m_sources.insert(filePath);
                break;
            }
        }
    }
}

bool CodeBuilder::isEmpty() const {
    return m_sources.empty();
}

void CodeBuilder::makeOutdated() {
    m_outdated = true;
}

bool CodeBuilder::isOutdated() const {
    return m_outdated;
}

QAbstractItemModel *CodeBuilder::classMap() const {
    return nullptr;
}
