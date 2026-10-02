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
#ifndef BUILDER_H
#define BUILDER_H

#include <objectsystem.h>
#include <object.h>
#include <log.h>

#include <stack>

#include <astring.h>

class Builder : public Object, public LogHandler {
    A_OBJECT(Builder, Object, General)
    A_METHODS(
        A_SLOT(Builder::onBuildSuccessful)
    )

public:
    Builder();

    void setPlatform(const TString &platform);
    void abort();
    int run();
    void setRecord(Log::LogTypes type, const char *record) override;

    bool package(const TString &target);

private:
    void pollImport();
    void onBuildSuccessful(bool result);
    void startNativeBuild();

private:
    std::stack<TString> m_platformsToBuild;
    int m_exitCode;
    bool m_finished;
};

#endif // BUILDER_H
