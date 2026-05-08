#include "os/filesystemwatcher.h"

#include <file.h>

#include <filesystem>
#include <atomic>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <fcntl.h>
#endif

class FileSystemWatcherPrivate {
public:
    struct FileInfo {
        TString path;
        std::filesystem::file_time_type lastWriteTime;
        uintmax_t size = 0;
    };

    FileSystemWatcherPrivate(FileSystemWatcher *ptr) :
            running(false),
            pollingMode(false),
            p_ptr(ptr) {

    }

    bool addPath(const TString &path) {
        std::lock_guard<std::mutex> lock(mutex);

        if(!File::exists(path)) {
            return false;
        }

        if(std::find(paths.begin(), paths.end(), path) != paths.end()) {
            return true;
        }

        paths.push_back(path);
        lastWriteTime[path] = getLastWriteTime(path);

        if(!pollingMode) {
            if(!initNativeWatcher(path)) {
                pollingMode = true;
            }
        }

        return true;
    }

    bool removePath(const TString &path) {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = std::find(paths.begin(), paths.end(), path);
        if(it != paths.end()) {
            size_t index = std::distance(paths.begin(), it);
            paths.erase(it);
            lastWriteTime.erase(path);

#ifdef _WIN32
            if(index < handles.size()) {
                CancelIo(handles[index]);
                CloseHandle(handles[index]);
                if (index < overlapped.size()) {
                    CloseHandle(overlapped[index].hEvent);
                }
                handles.erase(handles.begin() + index);
                if (index < overlapped.size())
                    overlapped.erase(overlapped.begin() + index);
            }
#elif defined(__linux__)
            for(auto& pair : watchDescriptors) {
                if (pair.second == path) {
                    inotify_rm_watch(inotifyFD, pair.first);
                    break;
                }
            }
            auto it_desc = std::find_if(watchDescriptors.begin(),
                                        watchDescriptors.end(),
                                        [&path](const auto& p) { return p.second == path; });
            if(it_desc != watchDescriptors.end()) {
                watchDescriptors.erase(it_desc);
            }
#elif defined(__APPLE__)
            if(paths.empty() && running) {
                if(watcherThread.joinable()) {
                    running = false;
                    cv.notify_all();
                    watcherThread.join();
                }
            }
#endif
            return true;
        }
        return false;
    }

    static std::filesystem::file_time_type getLastWriteTime(const TString &path) {
        try {
            if(File::exists(path)) {
                return std::filesystem::last_write_time(path.toStdString());
            }
        } catch (...) {}
        return std::filesystem::file_time_type();
    }

    void start() {
        if(running) {
            return;
        }

        running = true;

        if(pollingMode || !isNativeAvailable()) {
            watcherThread = std::thread(&FileSystemWatcherPrivate::pollingLoop, this);
        } else {
#ifdef _WIN32
            watcherThread = std::thread(&FileSystemWatcherPrivate::windowsWatchLoop, this);
#elif defined(__linux__)
            watcherThread = std::thread(&FileSystemWatcherPrivate::linuxWatchLoop, this);
#elif defined(__APPLE__)
            watcherThread = std::thread(&FileSystemWatcherPrivate::macosWatchLoop, this);
#endif
        }
    }

    void stop() {
        if(!running) {
            return;
        }

        running = false;

#ifdef _WIN32
        for(auto& ov : overlapped) {
            if(ov.hEvent) {
                SetEvent(ov.hEvent);
            }
        }
#elif defined(__APPLE__)
        cv.notify_all();
#endif

        if(watcherThread.joinable()) {
            watcherThread.join();
        }

#ifdef _WIN32
        for(auto h : handles) {
            CloseHandle(h);
        }
        for(auto& ov : overlapped) {
            CloseHandle(ov.hEvent);
        }
        handles.clear();
        overlapped.clear();
#elif defined(__linux__)
        if(inotifyFD >= 0) {
            close(inotifyFD);
            inotifyFD = -1;
        }
        watchDescriptors.clear();
#endif
    }

    bool isNativeAvailable() const {
#ifdef _WIN32
        return true;
#elif defined(__linux__)
        return true;
#elif defined(__APPLE__)
        return true;
#else
        return false;
#endif
    }

    bool initNativeWatcher(const TString &path) {
#ifdef _WIN32
        HANDLE hDir = CreateFileA(
            path.data(), FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

        if(hDir == INVALID_HANDLE_VALUE) {
            return false;
        }

        handles.push_back(hDir);
        overlapped.push_back(OVERLAPPED());
        overlapped.back().hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if(overlapped.back().hEvent == NULL) {
            CloseHandle(hDir);
            return false;
        }

        return true;
#elif defined(__linux__)
        if(inotifyFD < 0) {
            inotifyFD = inotify_init1(IN_NONBLOCK);
            if (inotifyFD < 0) {
                return false;
            }
        }

        int wd = inotify_add_watch(inotifyFD, path.data(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
        if(wd < 0) {
            return false;
        }

        watchDescriptors[wd] = path;
        return true;
#elif defined(__APPLE__)
        return true;
#else
        return false;
#endif
    }

    void checkDirectory(const TString &dirPath) {
        try {
            if(!File::exists(dirPath)) {
                // Directory removed
                p_ptr->directoryChanged(dirPath);
                return;
            }

            for(const auto &entry : File::list(dirPath)) {
                try {
                    auto currentTime = getLastWriteTime(entry);
                    auto lastTime = lastWriteTime[entry];

                    if (currentTime > lastTime) {
                        // File modified
                        p_ptr->fileChanged(entry);
                        lastWriteTime[entry] = currentTime;
                    }
                } catch (...) {}
            }
        } catch (const std::filesystem::filesystem_error& e) {}
    }

    void pollingLoop() {
        while(running) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (const auto& path : paths) {
                    checkDirectory(path);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

#ifdef _WIN32
    void windowsWatchLoop() {
        std::vector<char> buffer(65536);

        while (running) {
            for (size_t i = 0; i < paths.size() && running; ++i) {
                if (i >= handles.size()) continue;

                DWORD bytesReturned;
                if (ReadDirectoryChangesW(
                        handles[i], buffer.data(), static_cast<DWORD>(buffer.size()), TRUE,
                        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                        &bytesReturned, &overlapped[i], NULL)) {

                    DWORD waitResult = WaitForSingleObject(overlapped[i].hEvent, 1000);

                    if (waitResult == WAIT_OBJECT_0 && bytesReturned > 0) {
                        processWindowsEvents(i, buffer.data(), bytesReturned);
                    }

                    ResetEvent(overlapped[i].hEvent);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void processWindowsEvents(size_t index, void* buffer, DWORD bufferSize) {
        FILE_NOTIFY_INFORMATION* fni = static_cast<FILE_NOTIFY_INFORMATION*>(buffer);

        do {
            std::wstring wname(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
            std::string name(wname.begin(), wname.end());
            TString fullPath = paths[index] + "\\" + name;

            switch (fni->Action) {
                case FILE_ACTION_ADDED:
                case FILE_ACTION_REMOVED:
                case FILE_ACTION_MODIFIED:
                case FILE_ACTION_RENAMED_OLD_NAME:
                case FILE_ACTION_RENAMED_NEW_NAME:
                    p_ptr->fileChanged(fullPath);
                    break;
                default: break;
            }

            if (fni->NextEntryOffset == 0) break;
            fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<char*>(fni) + fni->NextEntryOffset);
        } while (true);
    }
#elif defined(__linux__)
    void linuxWatchLoop() {
        char buffer[4096];

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFD, &fds);

            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            int ret = select(inotifyFD + 1, &fds, NULL, NULL, &tv);

            if (ret > 0 && FD_ISSET(inotifyFD, &fds)) {
                ssize_t length = read(inotifyFD, buffer, sizeof(buffer));

                if (length > 0) {
                    processLinuxEvents(buffer, length);
                }
            }
        }
    }

    void processLinuxEvents(char* buffer, ssize_t length) {
        for (char* ptr = buffer; ptr < buffer + length; ) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(ptr);

            std::string eventPath;
            auto it = watchDescriptors.find(event->wd);
            if (it != watchDescriptors.end()) {
                eventPath = it->second;
                if (event->len > 0 && event->name[0] != '\0') {
                    eventPath += "/" + std::string(event->name);
                }
            }

            if(event->mask & IN_CREATE || event->mask & IN_DELETE || event->mask & IN_MODIFY ||
               event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
                if(!eventPath.empty()) {
                    p_ptr->fileChanged(fullPath);
                }
            }

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
#elif defined(__APPLE__)
    void macosWatchLoop() {
        auto lastState = getCurrentState();

        while(running) {
            {
                std::unique_lock<std::mutex> lock(mutex);
                if(cv.wait_for(lock, std::chrono::milliseconds(200), [this] { return !running; })) {
                    break;
                }
            }

            auto currentState = getCurrentState();
            detectChanges(lastState, currentState);
            lastState = std::move(currentState);
        }
    }

    std::unordered_map<TString, FileInfo> getCurrentState() {
        std::unordered_map<TString, FileInfo> state;

        for(const auto& path : paths) {
            if(File::exists(path)) {
                collectFiles(path, state);
            }
        }

        return state;
    }

    void collectFiles(const TString &path, std::unordered_map<TString, FileInfo> &state) {
        try {
            if(File::isDir(path)) {
                for(const auto& entry : File::list(path)) {
                    try {
                        FileInfo info;
                        info.path = entry;
                        info.lastWriteTime = std::filesystem::last_write_time(entry.toStdString());
                        info.size = std::filesystem::file_size(entry.toStdString());
                        state[info.path] = info;
                    } catch (...) {}
                }
            } else if(File::isFile(path)) {
                FileInfo info;
                info.path = path;
                info.lastWriteTime = std::filesystem::last_write_time(path.toStdString());
                info.size = std::filesystem::file_size(path.toStdString());
                state[info.path] = info;
            }
        } catch (...) {}
    }

    void detectChanges(const std::unordered_map<TString, FileInfo>& oldState,
                       const std::unordered_map<TString, FileInfo>& newState) {
        for(const auto& [path, info] : newState) {
            if(oldState.find(path) == oldState.end()) {
                // Created
                p_ptr->fileChanged(path);
            }
        }

        for(const auto& [path, info] : oldState) {
            if(newState.find(path) == newState.end()) {
                // Deleted
                p_ptr->fileChanged(path);
            }
        }

        for(const auto& [path, info] : newState) {
            auto it = oldState.find(path);
            if(it != oldState.end()) {
                if(info.lastWriteTime != it->second.lastWriteTime || info.size != it->second.size) {
                    // Modified
                    p_ptr->fileChanged(path);
                }
            }
        }
    }
#endif

public:
    std::vector<TString> paths;
    std::unordered_map<TString, std::filesystem::file_time_type> lastWriteTime;
    std::atomic<bool> running;
    std::atomic<bool> pollingMode;
    std::thread watcherThread;
    std::mutex mutex;
    std::condition_variable cv;

    FileSystemWatcher *p_ptr = nullptr;

#ifdef _WIN32
    std::vector<HANDLE> handles;
    std::vector<OVERLAPPED> overlapped;
#elif defined(__linux__)
    int inotifyFD = -1;
    std::unordered_map<int, TString> watchDescriptors;
#endif
};

FileSystemWatcher::FileSystemWatcher() :
        m_ptr(new FileSystemWatcherPrivate(this)) {

}

FileSystemWatcher::~FileSystemWatcher() {
    m_ptr->stop();
    delete m_ptr;
}

bool FileSystemWatcher::addPath(const TString &path) {
    return m_ptr->addPath(path);
}

bool FileSystemWatcher::addPaths(const StringList &paths) {
    bool allSuccess = true;
    for(const auto& path : paths) {
        if(!addPath(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool FileSystemWatcher::removePath(const TString &path) {
    return m_ptr->removePath(path);
}

void FileSystemWatcher::fileChanged(const TString &path) {
    emitSignal(_SIGNAL(fileChanged(TString)), path);
}

void FileSystemWatcher::directoryChanged(const TString &path) {
    emitSignal(_SIGNAL(directoryChanged(TString)), path);
}
