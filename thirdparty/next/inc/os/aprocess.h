/*
    This file is part of Thunder Next.

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

#ifndef PROCESS_H
#define PROCESS_H

#include <objectsystem.h>

class ProcessPrivate;

class ProcessEnvironment;

class NEXT_LIBRARY_EXPORT Process : public Object {
    A_OBJECT(Process, Object, Core)

    A_METHODS(
        A_SIGNAL(Process::errorOccurred),
        A_SIGNAL(Process::finished),
        A_SIGNAL(Process::readyReadStandardError),
        A_SIGNAL(Process::readyReadStandardOutput)
    )

public:

    enum State {
        NotRunning,
        Starting,
        Running,
        Finished
    };

    enum Error {
        FailedToStart,
        Crashed,
        Timedout,
        ReadError,
        WriteError,
        UnknownError
    };

public:
    Process();

    ~Process();

    bool start(const TString &program, const StringList &arguments);
    void terminate();
    void kill();

    bool waitForStarted(int timeoutMs = -1);
    bool waitForFinished(int timeoutMs = -1);

    void setWorkingDirectory(const TString &directory);

    void setProcessEnvironment(const ProcessEnvironment &environment);

    TString readAllStandardOutput();
    TString readAllStandardError();

    State state() const;
    int exitCode() const;
    bool isRunning() const;

    static bool openUrl(const TString &url);
    static bool startDetached(const TString &program, const StringList &arguments, const TString &workingDirectory, const ProcessEnvironment &environment);

public: // signals
    void errorOccurred(int error);

    void finished(int exitCode);

    void readyReadStandardError();

    void readyReadStandardOutput();

private:
    void readOutput();
    void monitorProcess();

private:
    ProcessPrivate *m_ptr;

};

#endif // PROCESS_H
