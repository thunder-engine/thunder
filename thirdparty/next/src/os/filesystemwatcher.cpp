#include "os/filesystemwatcher.h"

#include <file.h>
#include <url.h>

#include <filesystem>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
class FileSystemWatcherEvent : public Event {
public:
    FileSystemWatcherEvent(uint32_t type, const TString &path) :
            Event(type),
            m_path(path) {

    }

    TString m_path;

};

class FileSystemWatcherPrivate {
public:
    FileSystemWatcherPrivate(FileSystemWatcher *ptr) :
            running(false),
            p_ptr(ptr) {

    }

    bool addPath(const TString &path) {
        if(!File::exists(path)) {
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(mutex);
            if(std::find(paths.begin(), paths.end(), path) != paths.end()) {
                return true;
            }

            paths.push_back(path);
            lastWriteTime[path] = getLastWriteTime(path);
        }

        start();

        return true;
    }

    bool removePath(const TString &path) {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = std::find(paths.begin(), paths.end(), path);
        if(it != paths.end()) {
            paths.erase(it);
            lastWriteTime.erase(path);

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
        watcherThread = std::thread(&FileSystemWatcherPrivate::pollingLoop, this);
    }

    void stop() {
        if(!running) {
            return;
        }

        running = false;

        if(watcherThread.joinable()) {
            watcherThread.join();
        }
    }

    void checkDirectory(const TString &dirPath) {
        try {
            if(!File::exists(dirPath)) {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    paths.remove(dirPath);
                }
                TString path(Url(dirPath).absoluteDir());
                ObjectSystem::notify(p_ptr, new FileSystemWatcherEvent(Event::FileSystemWatcher, path));
                return;
            }

            for(const auto &entry : File::list(dirPath, true)) {
                try {
                    auto currentTime = getLastWriteTime(entry);
                    auto lastTime = lastWriteTime[entry];

                    if(currentTime > lastTime) {
                        ObjectSystem::notify(p_ptr, new FileSystemWatcherEvent(Event::FileSystemWatcher, entry));
                        lastWriteTime[entry] = currentTime;
                    }
                } catch (...) {}
            }
        } catch (const std::filesystem::filesystem_error &e) {}
    }

    void pollingLoop() {
        StringList pathsCopy;
        while(running) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                pathsCopy = paths;
            }

            for(const auto &path : pathsCopy) {
                checkDirectory(path);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

public:
    StringList paths;
    std::unordered_map<TString, std::filesystem::file_time_type> lastWriteTime;
    std::thread watcherThread;
    std::mutex mutex;

    bool running;

    FileSystemWatcher *p_ptr = nullptr;
};

/*!
    \class FileSystemWatcher
    \brief Watches filesystem paths and emits signals on changes.
    \since Next 1.0
    \inmodule OS

    `FileSystemWatcher` allows registering files or directories to be
    monitored. When a watched file or directory changes, the appropriate
    signal (`fileChanged` or `directoryChanged`) is emitted.
*/
FileSystemWatcher::FileSystemWatcher() :
        m_ptr(new FileSystemWatcherPrivate(this)) {

}

FileSystemWatcher::~FileSystemWatcher() {
    m_ptr->stop();
    delete m_ptr;
}


/*!
    Add a single \a path to the watch list (file or directory).

    Returns true if the path exists and was added (or already present).
*/
bool FileSystemWatcher::addPath(const TString &path) {
    return m_ptr->addPath(path);
}

/*!
    Add multiple \a paths to be watched.

    Attempts to add each path and returns true only if all additions succeed.
*/
bool FileSystemWatcher::addPaths(const StringList &paths) {
    bool allSuccess = true;
    for(const auto &path : paths) {
        if(!addPath(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

/*!
    Remove a single watched \a path.

    Returns true if the path was being watched and was removed.
*/
bool FileSystemWatcher::removePath(const TString &path) {
    return m_ptr->removePath(path);
}

/*!
    \brief Remove multiple watched \a paths.
*/
bool FileSystemWatcher::removePaths(const StringList &paths) {
    bool allSuccess = true;
    for(const auto &path : paths) {
        if(!removePath(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}
/*!
    Return a list of currently watched directories.
*/
StringList FileSystemWatcher::directories() const {
    StringList result;

    for(auto &it : m_ptr->paths) {
        if(File::isDir(it)) {
            result.push_back(it);
        }
    }

    return result;
}
/*!
    Return a list of currently watched files.
*/
StringList FileSystemWatcher::files() const {
    StringList result;

    for(auto &it : m_ptr->paths) {
        if(File::isFile(it)) {
            result.push_back(it);
        }
    }

    return result;
}
/*!
    Emit the `fileChanged` signal for the given path.
*/
void FileSystemWatcher::fileChanged(const TString &path) {
    emitSignal(_SIGNAL(fileChanged(TString)), path);
}
/*!
    Emit the `directoryChanged` signal for the given directory path.
*/
void FileSystemWatcher::directoryChanged(const TString &path) {
    emitSignal(_SIGNAL(directoryChanged(TString)), path);
}
/*!
    \internal
    Internal \a event handler: dispatch file/directory change events.

    Receives `FileSystemWatcherEvent` instances and emits the corresponding
    public signals.
*/
bool FileSystemWatcher::event(Event *event) {
    FileSystemWatcherEvent *ev = dynamic_cast<FileSystemWatcherEvent *>(event);
    if(ev) {
        TString path = ev->m_path;
        if(File::isDir(path)) { // Directory modified
            directoryChanged(path);
        } else { // File modified
            fileChanged(path);
        }
        return true;
    }

    return false;
}
