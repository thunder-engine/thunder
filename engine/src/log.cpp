#include "log.h"

#include <sstream>

static LogHandler *s_handler    = nullptr;
static Log::LogTypes s_logLevel = Log::ERR;

class LogPrivate {
public:
    std::stringstream       stream;
    Log::LogTypes           type;
};

/*!
    \class Log
    \brief The Log class provides an output stream for logging information.
    \inmodule Engine

    The Log is used whenever the developer needs to write out debugging or tracing information to a file or console.

    Common usecase:
    \code
    Log(Log::ERR) << "Loading level:" << 1;
    \endcode
*/
/*!
    \enum Log::LogTypes

    This enum defines the lavel of logging.

    \value ERR \c Error logging. For use with unrecoverable failures.
    \value WRN \c Warning logging. For use with recoverable failures.
    \value INF \c Informational logging. Should be desabled in release.
    \value DBG \c Debug logging. Should be desabled in release.
*/
/*!
    Constructs a log stream that writes to the handler for the message \a type.
*/
Log::Log(LogTypes type) :
        p_ptr(new LogPrivate()) {
    p_ptr->type = type;
}
/*!
    Flushes any pending data to be written and destroys the log stream.
*/
Log::~Log() {
    if(s_handler && p_ptr->type <= s_logLevel) {
        s_handler->setRecord(p_ptr->type, p_ptr->stream.str().c_str());
    }
    delete p_ptr;
}
/*!
    Set a new Log \a handler.
    This method can be used in case if a developer would need to move logging stream to someplace.
    For example to the console.
*/
void Log::overrideHandler(LogHandler *handler) {
    if(handler) {
        s_handler   = handler;
    }
}
/*!
    Returns LogHandler object if present; otherwise returns nullptr.
*/
LogHandler *Log::handler() {
    return s_handler;
}
/*!
    Set current log \a level output.
    Messages wich are below this \a level will be descarded.
*/
void Log::setLogLevel(LogTypes level) {
    s_logLevel  = level;
}
/*!
    Writes the boolean value, \a b, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(bool b) {
    p_ptr->stream << " "  << b;
    to_string(b);
    return *this;
}
/*!
    Writes the unsinged 8 bit integer value, \a c, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(uint8_t c) {
    p_ptr->stream << " "  << c;
    return *this;
}
/*!
    Writes the singed 8 bit integer value, \a c, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(int8_t c) {
    p_ptr->stream << " " << c;
    return *this;
}
/*!
    Writes the unsinged 16 bit integer value, \a s, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(uint16_t s) {
    p_ptr->stream << " " << s;
    return *this;
}
/*!
    Writes the singed 16 bit integer value, \a s, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(int16_t s) {
    p_ptr->stream << " " << s;
    return *this;
}
/*!
    Writes the unsinged 32 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(uint32_t i) {
    p_ptr->stream << " " << i;
    return *this;
}
/*!
    Writes the singed 32 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(int32_t i) {
    p_ptr->stream << " " << i;
    return *this;
}
/*!
    Writes the unsinged 64 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(uint64_t i) {
    p_ptr->stream << " " << i;
    return *this;
}
/*!
    Writes the singed 64 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(int64_t i) {
    p_ptr->stream << " " << i;
    return *this;
}
/*!
    Writes the float value, \a f, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(float f) {
    p_ptr->stream << " " << f;
    return *this;
}
/*!
    Writes the float value with double precision, \a d, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(double d) {
    p_ptr->stream << " " << d;
    return *this;
}
/*!
    Writes the '\\0'-terminated \a string, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(const char *string) {
    p_ptr->stream << " " << string;
    return *this;
}
