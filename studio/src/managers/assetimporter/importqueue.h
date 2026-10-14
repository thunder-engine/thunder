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
#ifndef IMPORTQUEUE_H
#define IMPORTQUEUE_H

#include <components/progressbar.h>

class ImportQueue : public ProgressBar {
    A_OBJECT(ImportQueue, ProgressBar, Components/UI)

    A_METHODS(
        A_SIGNAL(ImportQueue::importFinished)
    )

public:
    ImportQueue();

    void pollImport();

public: // signals
    void importFinished();

private:
    void startImport();
    void finishImport();

private:
    int m_totalImports;
    int m_processedImports;

    bool m_started;
    bool m_building;

};

#endif // IMPORTQUEUE_H
