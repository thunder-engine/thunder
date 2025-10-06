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

#ifndef APROCESS_H
#define APROCESS_H

#include <objectsystem.h>

class ProcessPrivate;

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
    bool waitForFinished(int timeoutMs = -1);

    TString readAllStandardOutput();
    TString readAllStandardError();

    State state() const;
    int exitCode() const;
    bool isRunning() const;

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

#endif // APROCESS_H
