#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

#include <global.h>

class NEXT_LIBRARY_EXPORT Event {
public:
    enum Type {
        Invalid = 0,
        MethodCall,
        TimerEvent,
        Destroy,
        LanguageChange,
        UserType = 100
    };

public:
    Event(uint32_t type);

    virtual ~Event();

    uint32_t type() const;

protected:
    uint32_t m_type;

};

#endif // EVENT_H
