#include "log.h"

static LogHandler *s_handler = nullptr;
static Log::LogTypes s_logLevel = Log::ERR;

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

    \value CRT \c Critical logging. For use with critical failures.
    \value ERR \c Error logging. For use with unrecoverable failures.
    \value WRN \c Warning logging. For use with recoverable failures.
    \value INF \c Informational logging. Should be desabled in release.
    \value DBG \c Debug logging. Should be desabled in release.
*/
/*!
    Constructs a log stream that writes to the handler for the message \a type.
*/
Log::Log(LogTypes type) :
        m_type(type) {
}
/*!
    Flushes any pending data to be written and destroys the log stream.
*/
Log::~Log() {
    if(s_handler && m_type <= s_logLevel) {
        s_handler->setRecord(m_type, m_stream.str().c_str());
    }
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
    m_stream << " "  << b;
    to_string(b);
    return *this;
}
/*!
    Writes the unsinged 8 bit integer value, \a c, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(unsigned char c) {
    m_stream << " "  << c;
    return *this;
}
/*!
    Writes the singed 8 bit integer value, \a c, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(char c) {
    m_stream << " " << c;
    return *this;
}
/*!
    Writes the unsinged 16 bit integer value, \a s, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(unsigned short s) {
    m_stream << " " << s;
    return *this;
}
/*!
    Writes the singed 16 bit integer value, \a s, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(short s) {
    m_stream << " " << s;
    return *this;
}
/*!
    Writes the unsinged 32 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(unsigned int i) {
    m_stream << " " << i;
    return *this;
}
/*!
    Writes the singed 32 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(int i) {
    m_stream << " " << i;
    return *this;
}
/*!
    Writes the unsinged 64 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(unsigned long long i) {
    m_stream << " " << i;
    return *this;
}
/*!
    Writes the singed 64 bit integer value, \a i, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(long long i) {
    m_stream << " " << i;
    return *this;
}
/*!
    Writes the float value, \a f, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(float f) {
    m_stream << " " << f;
    return *this;
}
/*!
    Writes the float value with double precision, \a d, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(double d) {
    m_stream << " " << d;
    return *this;
}
/*!
    Writes the '\\0'-terminated \a string, to the stream and returns a reference to the stream.
*/
Log &Log::operator<<(const char *string) {
    m_stream << " " << string;
    return *this;
}
