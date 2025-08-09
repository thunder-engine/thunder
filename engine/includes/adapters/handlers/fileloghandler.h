#ifndef DESKTOPLOGHANDLER_H
#define DESKTOPLOGHANDLER_H

#include <mutex>

#include <log.h>
#include <stdio.h>

class DesktopLogHandler : public LogHandler {
public:
    void setPath(TString &path) {
        m_path = path;
    }

protected:
    void setRecord(Log::LogTypes, const char *record) {
        if(!m_path.isEmpty()) {
            std::unique_lock<std::mutex> locker(m_mutex);
            FILE *fp = fopen((m_path + "/log.txt").data(), "a");
            if(fp) {
                fwrite(record, strlen(record), 1, fp);
                fwrite("\n", 1, 1, fp);
                fclose(fp);
            }
        }
    }

protected:
    TString m_path;

    std::mutex m_mutex;

};

#endif //DESKTOPLOGHANDLER_H
