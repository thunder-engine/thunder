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

#ifndef PIPELINE_H
#define PIPELINE_H

#include "resource.h"

class PipelineTask;

class ENGINE_EXPORT Pipeline : public Resource {
    A_OBJECT(Pipeline, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    struct Link {
        TString source;

        TString target;

        int output;

        int input;
    };

public:
    int renderTasksCount() const;

    TString renderTaskName(int index) const;

    int renderTasksLinksCount() const;

    Pipeline::Link renderTaskLink(int index) const;

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::vector<TString> m_renderTasks;

    std::vector<Link> m_renderTasksLinks;

};

#endif // PIPELINE_H
