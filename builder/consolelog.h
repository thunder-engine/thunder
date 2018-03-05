#ifndef CONSOLELOG_H
#define CONSOLELOG_H

#include <QObject>
#include <iostream>

#include <log.h>

class ConsoleLog : public QObject, public ILogHandler {
    Q_OBJECT

public:
    void                setRecord       (Log::LogTypes, const char *record) {
        std::cout << record << std::endl;
    }
};

#endif // CONSOLELOG_H
