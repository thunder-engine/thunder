#ifndef FILELOGHANDLER_H
#define FILELOGHANDLER_H

#include <mutex>

#include <log.h>
#include <engine.h>
#include <filesystem>

#include <stdio.h>

#define MAX_FILES 5

class DesktopLogHandler : public LogHandler {
public:
    DesktopLogHandler() {
        m_path = Engine::locationAppConfig().toStdString() + "/log.txt";

        // Remove oldest log
        std::string oldestFile = m_path + "." + std::to_string(MAX_FILES);
        if(std::filesystem::exists(oldestFile)) {
            std::filesystem::remove(oldestFile);
        }

        for(int i = MAX_FILES - 1; i > 0; --i) {
            std::string oldName = m_path + "." + std::to_string(i);
            std::string newName = m_path + "." + std::to_string(i + 1);
            if(std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }

        if(std::filesystem::exists(m_path)) {
            std::filesystem::rename(m_path, m_path + ".1");
        }
    }

protected:
    void setRecord(Log::LogTypes, const char *record) override {
        std::unique_lock<std::mutex> locker(m_mutex);
        FILE *fp = fopen(m_path.data(), "a");
        if(fp) {
            fwrite(record, strlen(record), 1, fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
        }
    }

protected:
    std::string m_path;

    std::mutex m_mutex;

};

#endif //FILELOGHANDLER_H
