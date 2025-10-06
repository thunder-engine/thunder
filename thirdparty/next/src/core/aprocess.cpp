#include "core/aprocess.h"

#include <chrono>
#include <thread>

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

class ProcessPrivate {
public:
    void cleanup() {
        if(m_monitorThread.joinable()) {
            m_monitorThread.join();
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

        m_state = Process::State::NotRunning;
    }

    TString m_stdoutBuffer;
    TString m_stderrBuffer;

    std::thread m_monitorThread;

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

    int m_exitCode = -1;
};

Process::Process() :
        m_ptr(new ProcessPrivate) {

}

Process::~Process() {
    if(m_ptr->m_state == State::Running) {
        kill();
    }
    m_ptr->cleanup();

    delete m_ptr;
}

bool Process::start(const TString &program, const StringList &arguments) {
    if(m_ptr->m_state == State::Running) {
        return false;
    }

    m_ptr->m_state = State::Starting;
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
        m_ptr->m_state = State::NotRunning;
        return false;
    }

    TString commandLine = program;
    commandLine += TString::join(arguments, " ");

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdinRead;
    si.hStdOutput = stdoutWrite;
    si.hStdError = stderrWrite;

    BOOL success = CreateProcessA(
        nullptr,
        const_cast<LPSTR>(commandLine.data()),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if(!success) {
        m_ptr->cleanup();
        m_ptr->m_state = State::NotRunning;

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
        cleanup();
        m_ptr->m_state = State::NotRunning;
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

        std::vector<char*> args;
        args.push_back(const_cast<char*>(program.data()));
        for(const auto &arg : arguments) {
            args.push_back(const_cast<char*>(arg.data()));
        }
        args.push_back(nullptr);

        execvp(program.data(), args.data());
    } else if(m_ptr->m_pid > 0) {
        close(m_ptr->m_stdoutPipe[1]); // Close write ends
        close(m_ptr->m_stderrPipe[1]);

        fcntl(m_ptr->m_stdoutPipe[0], F_SETFL, O_NONBLOCK);
        fcntl(m_ptr->m_stderrPipe[0], F_SETFL, O_NONBLOCK);

    } else {
        cleanup();
        m_ptr->m_state = State::NotRunning;

        errorOccurred(Error::FailedToStart);
        return false;
    }
#endif

    m_ptr->m_monitorThread = std::thread(&Process::monitorProcess, this);

    return true;
}

void Process::monitorProcess() {
    while(m_ptr->m_state == State::Running) {
        readOutput();

#ifdef _WIN32
        DWORD waitResult = WaitForSingleObject(m_ptr->m_processHandle, 100);
        if(waitResult == WAIT_OBJECT_0) {
            DWORD exitCode;
            GetExitCodeProcess(m_ptr->m_processHandle, &exitCode);
            m_ptr->m_exitCode = static_cast<int>(exitCode);
            m_ptr->m_state = State::Finished;

            finished(m_ptr->m_exitCode);
            break;
        }
#else
        if(m_ptr->m_pid > 0) {
            int status;
            pid_t result = waitpid(m_ptr->m_pid, &status, WNOHANG);
            if(result > 0) {
                if(WIFEXITED(status)) {
                    m_ptr->m_exitCode = WEXITSTATUS(status);
                } else if(WIFSIGNALED(status)) {
                    m_ptr->m_exitCode = WTERMSIG(status);
                }
                m_ptr->m_state = State::Finished;
                m_ptr->m_running = false;

                finished(m_ptr->m_exitCode);
                break;
            }
        }
#endif

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Process::readOutput() {
    char buffer[4096];

#ifdef _WIN32
    DWORD bytesRead;
    if(PeekNamedPipe(m_ptr->m_stdoutRead, nullptr, 0, nullptr, &bytesRead, nullptr) && bytesRead > 0) {
        if (ReadFile(m_ptr->m_stdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
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

void Process::terminate() {
    if(m_ptr->m_state != State::Running) {
        return;
    }

#ifdef _WIN32
    TerminateProcess(m_ptr->m_processHandle, 0);
#else
    ::kill(m_pid, SIGTERM);
#endif
}

void Process::kill() {
    if(m_ptr->m_state != State::Running) {
        return;
    }

#ifdef _WIN32
    TerminateProcess(m_ptr->m_processHandle, 1);
#else
    ::kill(m_pid, SIGKILL);
#endif
}

bool Process::waitForFinished(int timeoutMs) {
    if(m_ptr->m_state != State::Running) {
        return true;
    }

    if(m_ptr->m_monitorThread.joinable()) {
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
            m_ptr->m_monitorThread.join();
        }
    }

    return m_ptr->m_state == State::Finished;
}

TString Process::readAllStandardOutput() {
    TString result = m_ptr->m_stdoutBuffer;
    m_ptr->m_stdoutBuffer.clear();
    return result;
}

TString Process::readAllStandardError() {
    TString result = m_ptr->m_stderrBuffer;
    m_ptr->m_stderrBuffer.clear();
    return result;
}

Process::State Process::state() const {
    return m_ptr->m_state;
}

int Process::exitCode() const {
    return m_ptr->m_exitCode;
}

bool Process::isRunning() const {
    return m_ptr->m_state == State::Running;
}

void Process::errorOccurred(int error) {
    emitSignal(_SIGNAL(errorOccurred(int)), error);
}

void Process::finished(int exitCode) {
    emitSignal(_SIGNAL(finished(int)), exitCode);
}

void Process::readyReadStandardError() {
    emitSignal(_SIGNAL(readyReadStandardError()));
}

void Process::readyReadStandardOutput() {
    emitSignal(_SIGNAL(readyReadStandardOutput()));
}
