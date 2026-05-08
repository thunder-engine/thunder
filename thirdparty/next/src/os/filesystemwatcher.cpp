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
                ObjectSystem::notify(p_ptr, new FileSystemWatcherEvent(Event::UserType, path));
                return;
            }

            for(const auto &entry : File::list(dirPath)) {
                try {
                    auto currentTime = getLastWriteTime(entry);
                    auto lastTime = lastWriteTime[entry];

                    if(currentTime > lastTime) {
                        ObjectSystem::notify(p_ptr, new FileSystemWatcherEvent(Event::UserType, entry));
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
    for(const auto &path : paths) {
        if(!addPath(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool FileSystemWatcher::removePath(const TString &path) {
    return m_ptr->removePath(path);
}

bool FileSystemWatcher::removePaths(const StringList &paths) {
    bool allSuccess = true;
    for(const auto &path : paths) {
        if(!removePath(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

StringList FileSystemWatcher::directories() const {
    StringList result;

    for(auto &it : m_ptr->paths) {
        if(File::isDir(it)) {
            result.push_back(it);
        }
    }

    return result;
}

StringList FileSystemWatcher::files() const {
    StringList result;

    for(auto &it : m_ptr->paths) {
        if(File::isFile(it)) {
            result.push_back(it);
        }
    }

    return result;
}

void FileSystemWatcher::fileChanged(const TString &path) {
    emitSignal(_SIGNAL(fileChanged(TString)), path);
}

void FileSystemWatcher::directoryChanged(const TString &path) {
    emitSignal(_SIGNAL(directoryChanged(TString)), path);
}

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
