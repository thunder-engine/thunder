#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

#include <common.h>

class NEXT_LIBRARY_EXPORT Event {
public:
    enum Type {
        Invalid                 = 0,
        MethodCall,
        Timer,
        Delete,
        UserType                = 100
    };

public:
    Event                       (Type type);

    Type                        type                        () const;

protected:
    Type                        m_Type;
};

#endif // EVENT_H
