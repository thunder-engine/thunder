#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#include <engine.h>

class LogHandler;

class LogPrivate;

class NEXT_LIBRARY_EXPORT Log {
public:
    enum LogTypes {
        ERR             = 0,
        WRN             = 1,
        INF             = 2,
        DBG             = 3
    };

public:
    Log                 (LogTypes type);

    ~Log                ();

    static void         overrideHandler             (LogHandler *handler);

    static LogHandler  *handler                     ();

    static void         setLogLevel                 (LogTypes level);

    Log                &operator<<                  (bool b);

    Log                &operator<<                  (uint8_t c);
    Log                &operator<<                  (int8_t c);

    Log                &operator<<                  (uint16_t s);
    Log                &operator<<                  (int16_t s);

    Log                &operator<<                  (uint32_t i);
    Log                &operator<<                  (int32_t i);

    Log                &operator<<                  (uint64_t i);
    Log                &operator<<                  (int64_t i);

    Log                &operator<<                  (float f);
    Log                &operator<<                  (double d);

    Log                &operator<<                  (const char *s);

private:
    LogPrivate         *p_ptr;

};

class LogHandler {
public:
    virtual void setRecord (Log::LogTypes type, const char *record) = 0;

};

#endif // LOG_H
