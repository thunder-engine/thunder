#include "os/aprocess.h"

#include <chrono>
#include <thread>
#include <condition_variable>

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <sys/types.h>
    #include <fcntl.h>
#endif

#include "processenvironment.h"

class ProcessPrivate {
public:
    /*!
        \brief Release OS resources, join monitor thread and reset internal handles.

        This method ensures that any running monitor thread is joined, open
        handles/pipes are closed and internal state is reset to not running.
    */
    void cleanup() {
        if(m_monitorThread && m_monitorThread->joinable()) {
            m_monitorThread->join();
        }

    #ifdef _WIN32
        if(m_processHandle) CloseHandle(m_processHandle);
        if(m_threadHandle) CloseHandle(m_threadHandle);
        if(m_stdoutRead) CloseHandle(m_stdoutRead);
        if(m_stderrRead) CloseHandle(m_stderrRead);

        m_processHandle = m_threadHandle = nullptr;
        m_stdoutRead = m_stderrRead = nullptr;
    #else
        if(m_stdoutPipe[0] != -1) close(m_stdoutPipe[0]);
        if(m_stdoutPipe[1] != -1) close(m_stdoutPipe[1]);
        if(m_stderrPipe[0] != -1) close(m_stderrPipe[0]);
        if(m_stderrPipe[1] != -1) close(m_stderrPipe[1]);

        m_stdoutPipe[0] = m_stdoutPipe[1] = -1;
        m_stderrPipe[0] = m_stderrPipe[1] = -1;
        m_pid = -1;
    #endif

        setState(Process::State::NotRunning);
    }

    void setState(Process::State state) {
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_state = state;

            if(state == Process::State::Running) {
                m_processStarted = true;
            }
        }
        m_stateCondition.notify_all();
    }

    ProcessEnvironment m_processEnvironment;

    TString m_stdoutBuffer;
    TString m_stderrBuffer;

    TString m_workingDirectory;

    std::unique_ptr<std::thread> m_monitorThread;

#ifdef _WIN32
    HANDLE m_processHandle = nullptr;
    HANDLE m_threadHandle = nullptr;
    HANDLE m_stdoutRead = nullptr;
    HANDLE m_stderrRead = nullptr;
#else
    pid_t m_pid = -1;
    int m_stdoutPipe[2] = {-1, -1};
    int m_stderrPipe[2] = {-1, -1};
#endif

    Process::State m_state = Process::State::NotRunning;

    std::mutex m_stateMutex;
    std::condition_variable m_stateCondition;
    bool m_processStarted = false;

    int m_exitCode = -1;
};

/*!
    \module OS

    \title Next OS Module

    \brief Contains classes to work with operating system functionality.
*/

/*!
    \class Process
    \brief Class for managing external processes and interacting with their I/O.
    \since Next 1.0
    \inmodule OS

    This class provides a cross-platform interface for launching processes,
    reading standard output and error, waiting for start and finish, and
    managing the child process environment and working directory.

    Example:
    \code
        Process p;
        p.start("myapp", {"--option", "value"});
        if (p.waitForStarted(5000)) {
            // ...
        }
    \endcode
*/

/*!
    \enum Process::Error

    Error codes returned or emitted by the Process.

    \value FailedToStart \c Failed to launch the child process.
    \value Crashed \c The child process crashed or was terminated by a signal.
    \value Timedout \c An operation timed out.
    \value ReadError \c Error occurred while reading from process pipes.
    \value WriteError \c Error occurred while writing to process stdin.
    \value UnknownError \c An unspecified error occurred.
*/

/*!
    \enum Process::State

    Process lifecycle state.
    Describes the current lifecycle stage of the managed child process.

    \value NotRunning \c no process is running.
    \value Starting \c process start has been initiated.
    \value Running \c process is currently running.
    \value Finished \c process has terminated.
*/

/*!
    Construct a new Process object.
*/
Process::Process() :
    m_ptr(new ProcessPrivate) {

}

/*!
    Destructor; terminates running process and frees resources.
*/
Process::~Process() {
    if(m_ptr->m_state == State::Running) {
        kill();
    }
    m_ptr->cleanup();

    delete m_ptr;
}

/*!
    Start a process and begin monitoring it.

    Attempts to start the specified \a program with \a arguments. On success
    the process state becomes `Running` and a background monitor thread is
    spawned to read output and detect process termination.

    Returns true on success, false otherwise.
*/
bool Process::start(const TString &program, const StringList &arguments) {
    if(m_ptr->m_state == State::Running) {
        return false;
    }

    m_ptr->setState(State::Starting);
    m_ptr->cleanup();
    m_ptr->m_exitCode = -1;

#ifdef _WIN32
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE stdoutWrite;
    HANDLE stderrWrite;
    HANDLE stdinRead;
    HANDLE stdinWrite;

    if(!CreatePipe(&m_ptr->m_stdoutRead, &stdoutWrite, &saAttr, 0) ||
        !CreatePipe(&m_ptr->m_stderrRead, &stderrWrite, &saAttr, 0) ||
        !CreatePipe(&stdinRead, &stdinWrite, &saAttr, 0)) {

        m_ptr->cleanup();
        return false;
    }

    TString commandLine = program + " ";
    commandLine += TString::join(arguments, " ");

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdinRead;
    si.hStdOutput = stdoutWrite;
    si.hStdError = stderrWrite;

    LPVOID envBlock = nullptr;
    std::wstring envBlockData;
    if(!m_ptr->m_processEnvironment.envVars().empty()) {
        for(const auto &pair : m_ptr->m_processEnvironment.envVars()) {
            envBlockData += pair.first.toStdWString() + L"=" + pair.second.toStdWString() + L'\0';
        }
        envBlockData += L'\0'; // Double null terminator
        envBlock = (LPVOID)envBlockData.c_str();
    }

    LPCSTR workingDir = nullptr;
    if(!m_ptr->m_workingDirectory.isEmpty()) {
        workingDir = m_ptr->m_workingDirectory.data();
    }

    BOOL success = CreateProcessA(
        nullptr,
        const_cast<LPSTR>(commandLine.data()),
        nullptr,
        nullptr,
        TRUE,
        envBlock ? CREATE_UNICODE_ENVIRONMENT : 0,
        envBlock,
        workingDir,
        &si,
        &pi
    );

    if(!success) {
        m_ptr->cleanup();

        errorOccurred(Error::FailedToStart);
        return false;
    }

    m_ptr->m_processHandle = pi.hProcess;
    m_ptr->m_threadHandle = pi.hThread;

    CloseHandle(stdoutWrite);
    CloseHandle(stderrWrite);
    CloseHandle(stdinRead);
    CloseHandle(stdinWrite);
#else
    // Unix implementation
    if (pipe(m_ptr->m_stdoutPipe) == -1 || pipe(m_ptr->m_stderrPipe) == -1) {
        m_ptr->cleanup();
        return false;
    }

    m_ptr->m_pid = fork();

    if(m_ptr->m_pid == 0) {
        close(m_ptr->m_stdoutPipe[0]); // Close read end
        close(m_ptr->m_stderrPipe[0]);

        dup2(m_ptr->m_stdoutPipe[1], STDOUT_FILENO);
        dup2(m_ptr->m_stderrPipe[1], STDERR_FILENO);

        close(m_ptr->m_stdoutPipe[1]);
        close(m_ptr->m_stderrPipe[1]);

        if(!m_ptr->m_workingDirectory.isEmpty()) {
            chdir(m_ptr->m_workingDirectory.data());
        }

        std::vector<char*> args;
        args.push_back(const_cast<char*>(program.data()));
        for(const auto &arg : arguments) {
            args.push_back(const_cast<char*>(arg.data()));
        }
        args.push_back(nullptr);

        if(!m_ptr->m_processEnvironment.envVars().empty()) {
            std::vector<std::string> envArray;
            for(auto it : m_ptr->m_processEnvironment.envVars()) {
                envArray.push_back(it.first.toStdString() + "=" + it.second.toStdString());
            }
            std::vector<char*> envPtrs;
            for(auto &envStr : envArray) {
                envPtrs.push_back(const_cast<char*>(envStr.c_str()));
            }
            envPtrs.push_back(nullptr);

            execve(program.data(), args.data(), envPtrs.data());
        } else {
            execvp(program.data(), args.data());
        }
    } else if(m_ptr->m_pid > 0) {
        close(m_ptr->m_stdoutPipe[1]); // Close write ends
        close(m_ptr->m_stderrPipe[1]);

        fcntl(m_ptr->m_stdoutPipe[0], F_SETFL, O_NONBLOCK);
        fcntl(m_ptr->m_stderrPipe[0], F_SETFL, O_NONBLOCK);

    } else {
        m_ptr->cleanup();

        errorOccurred(Error::FailedToStart);
        return false;
    }
#endif

    m_ptr->setState(State::Running);

    m_ptr->m_monitorThread = std::make_unique<std::thread>(&Process::monitorProcess, this);

    return true;
}

/*!
    Open the given \a url using the system default handler.
*/
bool Process::openUrl(const TString &url) {
    StringList args;
#ifdef _WIN32
    TString command(url);
#elif __APPLE__
    TString command("open");
    args.push_back(url);
#else
    TString command("xdg-open");
    args.push_back(url);
#endif

    return startDetached(command, args, TString(), ProcessEnvironment::systemEnvironment());
}

/*!
    Start a detached process (no parent-child ties to this process).

    Starts \a program with \a arguments in a separate process group/session and
    returns immediately. Optionally sets the \a workingDirectory and the
    provided \a environment variables for the new process.

    Returns true if the detached process was launched successfully.
*/
bool Process::startDetached(const TString &program, const StringList &arguments, const TString &workingDirectory, const ProcessEnvironment &environment) {
#ifdef _WIN32
    TString commandLine = program + " ";
    commandLine += TString::join(arguments, " ");

    LPCSTR workingDir = nullptr;
    if(!workingDirectory.isEmpty()) {
        workingDir = workingDirectory.data();
    }

    LPVOID envBlock = nullptr;
    std::wstring envBlockData;
    if(!environment.envVars().empty()) {
        for(const auto &pair : environment.envVars()) {
            envBlockData += pair.first.toStdWString() + L"=" + pair.second.toStdWString() + L'\0';
        }
        envBlockData += L'\0'; // Double null terminator
        envBlock = (LPVOID)envBlockData.c_str();
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    DWORD creationFlags = CREATE_NO_WINDOW | DETACHED_PROCESS;
    if(envBlock) {
        creationFlags |= CREATE_UNICODE_ENVIRONMENT;
    }

    BOOL success = CreateProcessA(
        nullptr,                   // No module name (use command line)
        const_cast<LPSTR>(commandLine.data()),       // Command line
        nullptr,                   // Process handle not inheritable
        nullptr,                   // Thread handle not inheritable
        FALSE,                     // Set handle inheritance to FALSE
        creationFlags,             // Creation flags
        envBlock,                  // Environment block
        workingDir,                // Working directory
        &si,                       // Startup info
        &pi                        // Process information
    );

    if(success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    } else {
        HINSTANCE result = ShellExecuteW(
            nullptr,
            L"open",
            program.toStdWString().c_str(),
            commandLine.isEmpty() ? nullptr : commandLine.toStdWString().c_str(),
            workingDirectory.isEmpty() ? nullptr : workingDirectory.toStdWString().c_str(),
            SW_HIDE
        );

        return reinterpret_cast<intptr_t>(result) > 32;
    }

#else
    pid_t pid = fork();

    if(pid == 0) {
        setsid();

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        int nullfd = open("/dev/null", O_RDWR);
        if(nullfd != -1) {
            dup2(nullfd, STDIN_FILENO);
            dup2(nullfd, STDOUT_FILENO);
            dup2(nullfd, STDERR_FILENO);
            close(nullfd);
        }

        if(!workingDirectory.isEmpty()) {
            if(chdir(workingDirectory.data()) == -1) {
                return false;
            }
        }

        std::vector<char*> args;
        args.push_back(const_cast<char*>(program.data()));
        for(const auto &arg : arguments) {
            args.push_back(const_cast<char*>(arg.data()));
        }
        args.push_back(nullptr);

        if(!environment.envVars().empty()) {
            std::vector<std::string> envArray;
            for(auto it : environment.envVars()) {
                envArray.push_back(it.first.toStdString() + "=" + it.second.toStdString());
            }
            std::vector<char*> envPtrs;
            for(auto &envStr : envArray) {
                envPtrs.push_back(const_cast<char*>(envStr.c_str()));
            }
            envPtrs.push_back(nullptr);

            execve(program.data(), args.data(), envPtrs.data());
        } else {
            execvp(program.data(), args.data());
        }

    } else if (pid > 0) {
        return true;
    }
#endif

    return false;
}

/*!
    \internal
    Background thread function that monitors the child process.
*/
void Process::monitorProcess() {
    while(m_ptr->m_state == State::Running) {
#ifdef _WIN32
        DWORD waitResult = WaitForSingleObject(m_ptr->m_processHandle, 100);
        if(waitResult == WAIT_OBJECT_0) {
            DWORD exitCode;
            GetExitCodeProcess(m_ptr->m_processHandle, &exitCode);
            m_ptr->m_exitCode = static_cast<int>(exitCode);
            m_ptr->setState(State::Finished);

            readOutput();
            finished(m_ptr->m_exitCode);
            break;
        } else if (waitResult == WAIT_TIMEOUT) {
            readOutput();
        }
#else
        readOutput();

        if(m_ptr->m_pid > 0) {
            int status;
            pid_t result = waitpid(m_ptr->m_pid, &status, WNOHANG);
            if(result > 0) {
                if(WIFEXITED(status)) {
                    m_ptr->m_exitCode = WEXITSTATUS(status);
                } else if(WIFSIGNALED(status)) {
                    m_ptr->m_exitCode = WTERMSIG(status);
                }
                m_ptr->setState(State::Finished);

                readOutput();
                finished(m_ptr->m_exitCode);
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
}

/*!
    \internal
    Read any available data from child stdout/stderr into buffers.
*/
void Process::readOutput() {
    char buffer[4096];

#ifdef _WIN32
    DWORD bytesRead;
    if(PeekNamedPipe(m_ptr->m_stdoutRead, nullptr, 0, nullptr, &bytesRead, nullptr) && bytesRead > 0) {
        if(ReadFile(m_ptr->m_stdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            m_ptr->m_stdoutBuffer += buffer;

            readyReadStandardOutput();
        }
    }

    if(PeekNamedPipe(m_ptr->m_stderrRead, nullptr, 0, nullptr, &bytesRead, nullptr) && bytesRead > 0) {
        if(ReadFile(m_ptr->m_stderrRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            m_ptr->m_stderrBuffer += buffer;

            readyReadStandardError();
        }
    }
#else
    ssize_t bytesRead = read(m_ptr->m_stdoutPipe[0], buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        m_ptr->m_stdoutBuffer += buffer;

        readyReadStandardOutput();
    }

    bytesRead = read(m_ptr->m_stderrPipe[0], buffer, sizeof(buffer) - 1);
    if(bytesRead > 0) {
        buffer[bytesRead] = '\0';
        m_ptr->m_stderrBuffer += buffer;

        readyReadStandardError();
    }
#endif
}

/*!
    Sends a termination request to the process. This is a graceful request;
    it does not force immediate termination.
*/
void Process::terminate() {
    if(m_ptr->m_state != State::Running) {
        return;
    }

#ifdef _WIN32
    TerminateProcess(m_ptr->m_processHandle, 0);
#else
    ::kill(m_ptr->m_pid, SIGTERM);
#endif
}

/*!
    This unconditionally terminates the process. Use only when `terminate()`
    did not cause the process to exit in a timely manner.
*/
void Process::kill() {
    if(m_ptr->m_state != State::Running) {
        return;
    }

#ifdef _WIN32
    TerminateProcess(m_ptr->m_processHandle, 1);
#else
    ::kill(m_ptr->m_pid, SIGKILL);
#endif
}

/*!
    Wait for the process to start.

    Blocks until the process has started or the optional \a timeoutMs
    milliseconds have elapsed. A negative \a timeoutMs means wait forever.
    Returns true if the process started successfully.
*/
bool Process::waitForStarted(int timeoutMs) {
    if(m_ptr->m_state == State::Running) {
        return true;
    }

    if(m_ptr->m_state == State::NotRunning || m_ptr->m_state == State::Finished) {
        return false;
    }

    std::unique_lock<std::mutex> lock(m_ptr->m_stateMutex);

    if(timeoutMs < 0) {
        m_ptr->m_stateCondition.wait(lock, [this]() {
            return m_ptr->m_processStarted || m_ptr->m_state == State::NotRunning || m_ptr->m_state == State::Finished;
        });
    } else {
        auto result = m_ptr->m_stateCondition.wait_for(lock,
                                        std::chrono::milliseconds(timeoutMs),
                                        [this]() {
                                            return m_ptr->m_processStarted || m_ptr->m_state == State::NotRunning || m_ptr->m_state == State::Finished;
                                        });

        if(!result) {
            return false;
        }
    }

    return m_ptr->m_processStarted && m_ptr->m_state == State::Running;
}

/*!
    Wait for the process to finish.

    Blocks until the monitored process has finished or until \a timeoutMs
    milliseconds have passed. A negative \a timeoutMs will wait without
    polling and join the monitor thread.

    Returns true if the process has finished.
*/
bool Process::waitForFinished(int timeoutMs) {
    if(m_ptr->m_state != State::Running) {
        return true;
    }

    if(m_ptr->m_monitorThread && m_ptr->m_monitorThread->joinable()) {
        if(timeoutMs > 0) {
            auto start = std::chrono::steady_clock::now();
            while(m_ptr->m_state == State::Running) {
                if(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count() > timeoutMs) {
                    return false; // Timeout
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else {
            m_ptr->m_monitorThread->join();
        }
    }

    return m_ptr->m_state == State::Finished;
}

/*!
    Set the working \a directory for the process to be started.
*/
void Process::setWorkingDirectory(const TString &directory) {
    m_ptr->m_workingDirectory = directory;
}

/*!
    Set \a environment variables for the child process.
*/
void Process::setProcessEnvironment(const ProcessEnvironment &environment) {
    m_ptr->m_processEnvironment = environment;
}

/*!
    Return and clear the accumulated standard output buffer.

    Returns all data read from the child process's stdout since the last
    call and clears the internal buffer.
*/
TString Process::readAllStandardOutput() {
    TString result = m_ptr->m_stdoutBuffer;
    m_ptr->m_stdoutBuffer.clear();
    return result;
}

/*!
    Return and clear the accumulated standard error buffer.

    Returns all data read from the child process's stderr since the last
    call and clears the internal buffer.
*/
TString Process::readAllStandardError() {
    TString result = m_ptr->m_stderrBuffer;
    m_ptr->m_stderrBuffer.clear();
    return result;
}

/*!
    Get the current process state.

    Returns one of the `Process::State` enumeration values describing the
    lifecycle state of the child process.
*/
Process::State Process::state() const {
    return m_ptr->m_state;
}

/*!
    Retrieve the exit code of the finished process.

    If the process has finished, returns its exit code; otherwise returns
    the last known exit code value (or -1 if unknown).
*/
int Process::exitCode() const {
    return m_ptr->m_exitCode;
}

/*!
    Check whether the process is currently running.

    Convenience helper returning true when the internal state is
    `Process::State::Running`.
*/
bool Process::isRunning() const {
    return m_ptr->m_state == State::Running;
}

/*!
    Emit an signal with the given \a error code.

    The \a error parameter is one of the Process::Error enumeration values.
*/
void Process::errorOccurred(int error) {
    emitSignal(_SIGNAL(errorOccurred(int)), error);
}

/*!
    Emit signal when the process exits.

    The \a exitCode parameter contains the child's exit status.
*/
void Process::finished(int exitCode) {
    emitSignal(_SIGNAL(finished(int)), exitCode);
}

/*!
    Emit when stderr data is available.
*/
void Process::readyReadStandardError() {
    emitSignal(_SIGNAL(readyReadStandardError()));
}

/*!
    Emit when stdout data is available.
*/
void Process::readyReadStandardOutput() {
    emitSignal(_SIGNAL(readyReadStandardOutput()));
}
