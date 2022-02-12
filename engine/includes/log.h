#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#include <engine.h>

class LogHandler;

class LogPrivate;

class ENGINE_EXPORT Log {
public:
    enum LogTypes {
        CRT = 0,
        ERR,
        WRN,
        INF,
        DBG,
    };

public:
    Log(LogTypes type);

    ~Log();

    static void overrideHandler(LogHandler *handler);

    static LogHandler *handler();

    static void setLogLevel(LogTypes level);

    Log &operator<<(bool b);

    Log &operator<<(unsigned char c);
    Log &operator<<(char c);

    Log &operator<<(unsigned short s);
    Log &operator<<(short s);

    Log &operator<<(unsigned int i);
    Log &operator<<(int i);

    Log &operator<<(unsigned long long i);
    Log &operator<<(long long i);

    Log &operator<<(float f);
    Log &operator<<(double d);

    Log &operator<<(const char *s);

private:
    LogPrivate *p_ptr;

};

#define aCritical()Log(Log::CRT)
#define aError()   Log(Log::ERR)
#define aWarning() Log(Log::WRN)
#define aInfo()    Log(Log::INF)
#define aDebug()   Log(Log::DBG)

class LogHandler {
public:
    virtual void setRecord(Log::LogTypes type, const char *record) = 0;

};

#endif // LOG_H
