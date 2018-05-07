#include "log.h"

#include <sstream>

static ILogHandler *s_handler    = nullptr;
static Log::LogTypes s_logLevel  = Log::ERR;

class LogPrivate {
public:
    std::stringstream       stream;
    Log::LogTypes           type;
};

Log::Log(LogTypes type) :
        p_ptr(new LogPrivate()) {
    p_ptr->type = type;
}

Log::~Log() {
    if(s_handler && p_ptr->type <= s_logLevel) {
        s_handler->setRecord(p_ptr->type, p_ptr->stream.str().c_str());
    }
    delete p_ptr;
}

void Log::overrideHandler(ILogHandler *handler) {
    if(handler) {
        s_handler   = handler;
    }
}

void Log::setLogLevel(LogTypes lvl) {
    s_logLevel  = lvl;
}

Log &Log::operator<<(bool b) {
    p_ptr->stream << " "  << b;
    to_string(b);
    return *this;
}

Log &Log::operator<<(uint8_t c) {
    p_ptr->stream << " "  << c;
    return *this;
}

Log &Log::operator<<(int8_t c) {
    p_ptr->stream << " " << c;
    return *this;
}

Log &Log::operator<<(uint16_t s) {
    p_ptr->stream << " " << s;
    return *this;
}

Log &Log::operator<<(int16_t s) {
    p_ptr->stream << " " << s;
    return *this;
}

Log &Log::operator<<(uint32_t i) {
    p_ptr->stream << " " << i;
    return *this;
}

Log &Log::operator<<(int32_t i) {
    p_ptr->stream << " " << i;
    return *this;
}

Log &Log::operator<<(uint64_t i) {
    p_ptr->stream << " " << i;
    return *this;
}

Log &Log::operator<<(int64_t i) {
    p_ptr->stream << " " << i;
    return *this;
}

Log &Log::operator<<(float f) {
    p_ptr->stream << " " << f;
    return *this;
}

Log &Log::operator<<(double d) {
    p_ptr->stream << " " << d;
    return *this;
}

Log &Log::operator<<(const char *s) {
    p_ptr->stream << " " << s;
    return *this;
}
