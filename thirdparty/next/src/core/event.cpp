/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
    \value FileSystemWatcher \c Some file or directory was chanded.
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
