#ifndef CONSOLELOG_H
#define CONSOLELOG_H

#include <QObject>
#include <QDebug>

#include <log.h>

class ConsoleLog : public QObject, public ILogHandler {
    Q_OBJECT

public:
    void                setRecord       (Log::LogTypes type, const char *record) {
        switch(type) {
            case Log::DBG: QDebug(QtDebugMsg) << record; break;
            case Log::INF: QDebug(QtInfoMsg) << record; break;
            case Log::WRN: QDebug(QtWarningMsg) << record; break;
            default:  QDebug(QtCriticalMsg) << record; break;
        }
    }
};

#endif // CONSOLELOG_H
