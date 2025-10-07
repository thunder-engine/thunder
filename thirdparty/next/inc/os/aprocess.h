/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
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
