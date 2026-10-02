/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "builder.h"

#include <log.h>
#include <url.h>
#include <file.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/assetmanager.h>
#include <editor/nativecodebuilder.h>

#include <compat/zip.h>

#include <chrono>
#include <iostream>
#include <thread>

Builder::Builder() :
        m_importStarted(false),
        m_waitingForNative(false),
        m_nativeBuildRequested(false),
        m_exitCode(0) {
    Object::connect(Editor::assets(), _SIGNAL(buildSuccessful(bool)), this, _SLOT(onBuildSuccessful(bool)));
}

void Builder::pollImport() {
    AssetManager *manager = Editor::assets();
    while(m_importStarted && m_exitCode == 0) {
        while(manager->pendingImportCount() > 0 && m_exitCode == 0) {
            manager->importNext();
        }
        if(m_exitCode != 0) {
            m_importStarted = false;
            break;
        }
        if(manager->finishImport()) {
            m_importStarted = false;
            onImportFinished();
            break;
        }
    }

    processBuildResults();
    if(m_exitCode != 0) {
        return;
    }

}

int Builder::run() {
    while(m_importStarted || m_waitingForNative) {
        pollImport();
        if(m_exitCode != 0 && !m_waitingForNative) {
            break;
        }
        if(m_importStarted || m_waitingForNative) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    return m_exitCode;
}

void Builder::abort() {
    m_exitCode = 1;
}

void Builder::setRecord(Log::LogTypes type, const char *record) {
    const char *level = "";
    switch(type) {
        case Log::CRT: level = "[ critical ]"; break;
        case Log::ERR: level = "[ error ]"; break;
        case Log::WRN: level = "[ warning ]"; break;
        case Log::INF: level = "[ info ]"; break;
        case Log::DBG: level = "[ debug ]"; break;
        default: break;
    }

    std::cout << level << record << std::endl;
    if(type <= Log::ERR) {
        abort();
    }
}

void Builder::setPlatform(const TString &platform) {
    if(m_exitCode != 0) {
        return;
    }

    ProjectSettings *project = Editor::project();
    Editor::settings()->loadSettings();
    if(platform.isEmpty()) {
        for(const TString &it : project->platforms()) {
            m_platformsToBuild.push(it);
        }
    } else {
        m_platformsToBuild.push(platform);
    }

    if(!m_platformsToBuild.empty()) {
        project->setCurrentPlatform(m_platformsToBuild.top());
        m_platformsToBuild.pop();

        NativeCodeBuilder *builder = project->currentBuilder();
        if(builder) {
            builder->convertFile(nullptr);
        }

        m_importStarted = true;
        Editor::assets()->rescan();
        pollImport();
    }
}

bool Builder::package(const TString &target) {
    TString pak = target + "/base.pak";

    aInfo() << "Packaging Assets to:" << pak << target;

    zipFile zf = zipOpen(pak.data(), 0);
    if(!zf) {
        aError() << "Can't open package.";
        return false;
    }

    StringList list(File::list(Editor::project()->importPath()));
    for(auto &it : list) {
        if(File::isFile(it)) {
            Url info(it);

            TString origin = Editor::assets()->uuidToPath(info.baseName());
            aInfo() << "\tCoping:" << origin.data();

            File inFile(it);
            if(!inFile.open(File::Read)) {
                zipClose(zf, nullptr);
                aError() << "Can't open input file.";
                return false;
            }

            zip_fileinfo zi = {0};
            zipOpenNewFileInZip(zf, info.name().data(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_NO_COMPRESSION);

            ByteArray data(inFile.readAll());
            inFile.close();

            zipWriteInFileInZip(zf, data.data(), data.size());
            zipCloseFileInZip(zf);
        }
    }

    zipClose(zf, nullptr);

    aInfo() << "Packaging Done.";
    return true;
}

void Builder::onImportFinished() {
    if(m_exitCode != 0) {
        return;
    }
    startNativeBuild();
}

void Builder::onBuildSuccessful(bool buildResult) {
    if(!m_nativeBuildRequested) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_buildResultsMutex);
    m_buildResults.push(buildResult);
}

void Builder::processBuildResults() {
    while(true) {
        bool result;
        {
            std::lock_guard<std::mutex> lock(m_buildResultsMutex);
            if(m_buildResults.empty()) {
                break;
            }
            result = m_buildResults.front();
            m_buildResults.pop();
        }
        m_waitingForNative = false;
            m_nativeBuildRequested = false;
            if(m_exitCode == 0) {
                handleNativeBuildSuccessful(result);
            }
    }
}

void Builder::startNativeBuild() {
    ProjectSettings *project = Editor::project();
    NativeCodeBuilder *builder = project->currentBuilder();
    if(!builder) {
        m_exitCode = 1;
        return;
    }

    if(builder->packagingMode() == NativeCodeBuilder::Before) {
        package(project->cachePath() + "/" + project->currentPlatformName());
    }
    if(m_exitCode != 0) {
        return;
    }

    m_nativeBuildRequested = true;
    m_waitingForNative = builder->buildProject();
    if(!m_waitingForNative) {
        m_nativeBuildRequested = false;
        m_exitCode = 1;
    }
}

void Builder::handleNativeBuildSuccessful(bool buildResult) {
    if(!buildResult) {
        m_exitCode = 1;
        return;
    }

    ProjectSettings *project = Editor::project();
    TString targetPath = project->targetPath() + "/" + project->currentPlatformName();

    if(!File::exists(targetPath) && !File::mkPath(targetPath)) {
        aDebug() << "Unable to create build directory at:" << targetPath;
    }

    // Clean install dir
    for(auto &it : File::list(targetPath)) {
        File::remove(it);
    }

    bool result = true;
    for(const TString &it : project->artifacts()) {
        result &= File::copy(it, targetPath + "/" + Url(it).name());
    }

    if(result) {
        aInfo() << "New build copied to:" << targetPath;

        // Package after
        NativeCodeBuilder *builder = project->currentBuilder();
        if(builder && builder->packagingMode() == NativeCodeBuilder::After) {
            // Package right to install dir
            package(targetPath);
        }

        if(m_exitCode != 0) {
            return;
        }

        if(!m_platformsToBuild.empty()) {
            project->setCurrentPlatform(m_platformsToBuild.top());
            m_platformsToBuild.pop();
            m_importStarted = true;
            Editor::assets()->rescan();
            pollImport();

            return;
        }
    }

    m_exitCode = result ? 0 : 1;
}
