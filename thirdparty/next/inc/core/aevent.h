#ifndef AEVENT_H
#define AEVENT_H

#include <stdint.h>

#include <acommon.h>

class NEXT_LIBRARY_EXPORT AEvent {
public:
    enum Type {
        Invalid                 = 0,
        MethodCall,
        Delete,
        UserType
    };

public:
    AEvent                      (Type type);

    Type                        type                        () const;

protected:
    Type                        m_Type;
};

#endif // AEVENT_H
