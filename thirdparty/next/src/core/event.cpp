#include "core/event.h"
/*!
    \class Event
    \brief The Event class is the base calss for all event classes.
    \since Next 1.0
    \inmodule Core

    Objects processing events by their virtual Object::event() function called.
    This function can be reimplemented in subclasses to add additional event handling or change existing.

    Base Event contain only event type parameter. Subclasses of Event may contain additional parameters to describe particular events.

    \sa Object::event()
*/
/*!
    \enum Event::Type

    This enum type defines base event types and can be extended by the user defined types.
    User Defined type of Event should be bigger than Event::UserType.

    \value Invalid \c Invalid event.
    \value MethodCall \c Receiver object should invoke method (MethodCallEvent).
    \value TimerEvent \c Timer event (TimerEvent).
    \value Destroy \c Reseiver object must be deleted immediately.
    \value LanguageChange \c The application translation changed.
    \value UserType \c User defined event.
*/
/*!
    Constructs an Event with \a type of event.
*/
Event::Event(uint32_t type) :
        m_type(type) {
    PROFILE_FUNCTION();
}

Event::~Event() {

}

/*!
    Returns type of event.
*/
uint32_t Event::type() const {
    PROFILE_FUNCTION();
    return m_type;
}
