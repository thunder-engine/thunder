/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#include "core/objectsystem.h"
#include "core/uri.h"

#include <mutex>
#include <iostream>
#include <sstream>

static inline std::mutex &lockMutex(const Object *o) {
    static std::mutex s_mutexPool[131];
    return s_mutexPool[uint64_t(o) % sizeof(s_mutexPool) / sizeof(std::mutex)];
}

/*!
    \module Core

    \title Next Core Module

    \brief Contains base classes for interobject collaboration and introspection.
*/

/*!
    \typedef Object::ObjectList

    Synonym for list<Object *>.
*/

/*!
    \typedef Object::LinkList

    Synonym for list<Link *>.
*/

inline bool operator==(const Object::Link &left, const Object::Link &right) {
    bool result = true;
    result &= (left.sender == right.sender);
    result &= (left.receiver == right.receiver);
    result &= (left.signal == right.signal);
    result &= (left.method == right.method);
    return result;
}

Object::Link::Link() :
    sender(nullptr),
    signal(-1),
    receiver(nullptr),
    method(-1) {

}

/*!
    \class Object
    \brief The Object class is the base calss for all object classes.
    \since Next 1.0
    \inmodule Core

    The object is the central part of the Next library.
    For communication between objects two mechanisms was implemented the signals and slots also event based approach.
    To connect two objects between use connect() method and the sender object  will notify the receiver object about necessary events.

    Objects can be organized into an object trees. Each object can have an unlimited number of children objects.
    When you assign parent to an object it automatically add itself to the parent children list. Parent object takes ownership of the child object.
    This means that the child will be automatically deleted if the parent object is deleted.
    Child object can be found in hierarchy of objects by path or by the type using find(), findChild() or findChildren().

    Each Object has name() and this name must be unique in space of object level in hierarchy i.e. parent with name "House" can't has two childs with name "Roof".
    This names is used to reach objects by its paths land.find("House/Roof") will return "Roof" object.

    Each Object has MetaObject declaration. MetaObject system can be used to declare and retrieve structure of object at runtime.

    Based on Actor model the object can't be copied only clone().

    \sa MetaObject
*/
/*!
    \fn template<typename T> T Object::findChild(bool recursive)

    Returns the first child of this object that can be cast to type T.
    The search is performed recursively, unless \a recursive option is false.

    Returns nullptr if no such object.

    \sa find(), findChildren()
*/
/*!
    \fn template<typename T> list<T> Object::findChildren(bool recursive)

    Returns all children of this object that can be cast to type T.
    The search is performed recursively, unless \a recursive option is false.

    Returns empty list if no such objects.

    \sa find(), findChildren()
*/
/*!
    \macro A_OBJECT(Class, Super)
    \relates Object

    This macro creates member functions to create MetaObject's.

    \a Class must be current class name
    \a Super must be class name of parent

    Example:
    \code
        class MyObject : public Object {
            A_OBJECT(MyObject, Object)
        };
    \endcode

    And then:
    \code
        MetaObject *meta = MyObject::metaClass();
    \endcode
*/
/*!
    \macro A_REGISTER(Class, Super, Group)
    \relates Object

    This macro creates member functions for registering and unregistering in ObjectSystem factory.

    \a Class must be current class name
    \a Super must be class name of parent
    \a Group could be any and used to help manage factories

    Example:
    \code
        class MyObject : public Object {
            A_REGISTER(MyObject, Object, Core)
        };

        ....
        MyObject::registerClassFactory();
    \endcode

    And then:
    \code
        MyObject *oject = ObjectSystem::createObject<MyObject>();
    \endcode

    \note Also it's includes A_OBJECT() macro

    \sa ObjectSystem::objectCreate(), A_OBJECT()
*/
/*!
    \macro A_OVERRIDE(Class, Super, Group)
    \relates Object

    This macro works pertty mutch the same as A_REGISTER() macro but with little difference.
    It's override \a Super factory in ObjectSystem by own routine.
    And restore original state when do unregisterClassFactory().

    This macro can be used to implement polymorphic behavior for factories.

    \a Class must be current class name
    \a Super must be class name of parent
    \a Group could be any and used to help manage factories

    Example:
    \code
        class MyObject : public Object {
            A_OVERRIDE(MyObject, Object, Core)
        };

        ...
        MyObject::registerClassFactory();
    \endcode

    And then:
    \code
        Object *oject = ObjectSystem::createObject<Object>();
        MyObject *myObject = dynamic_cast<MyObject *>(oject);
        if(myObject) {
            ...
        }
    \endcode

    \note Also it's includes A_OBJECT macro

    \sa ObjectSystem::objectCreate(), A_REGISTER(), A_OBJECT
*/
/*!
    \macro A_METHODS()
    \relates Object

    This macro is a container to keep information about included methods.

    There are three possible types of methods:
    \table
    \header
        \li Type
        \li Description
    \row
        \li A_SIGNAL
        \li Method without impelementation can't be invoked. Used for Signals and Slots mechanism.
    \row
        \li A_METHOD
        \li Standard method can be invoked. Used for general porposes.
    \row
        \li A_SLOT
        \li Very similar to A_METHOD but with special flag to be used for Signal-Slot mechanism.
    \endtable


    For example declare an introspectable class.
    \code
        class MyObject : public Object {
            A_REGISTER(MyObject, Object, General)

            A_METHODS(
                A_METHOD(foo)
                A_SIGNAL(signal)
            )

        public:
            void foo() { }

            void signal();
        };
    \endcode

    And then:
    \code
        MyObject obj;
        const MetaObject *meta = obj.metaObject();

        int index = meta->indexOfMethod("foo");
        if(index > -1) {
            MetaMethod method = meta->method(index);
            if(method.isValid() {
                Variant value;
                method.invoke(&obj, value, 0, nullptr); // Will call MyObject::foo method
            }
        }
    \endcode
*/
/*!
    Constructs an object.

    By default Object create without parent to assign the parent object use setParent().
*/
Object::Object() :
    m_parent(nullptr),
    m_currentSender(nullptr),
    m_system(nullptr),
    m_uuid(0),
    m_cloned(0) {
    PROFILE_FUNCTION();

    setUUID(ObjectSystem::generateUUID());
}

Object::~Object() {
    PROFILE_FUNCTION();

    emitSignal(_SIGNAL(destroyed()));

    if(m_system) {
         m_system->removeObject(this);
    }
    {
        std::lock_guard<std::mutex> locker(lockMutex(this));
        while(!m_eventQueue.empty()) {
            delete m_eventQueue.front();
            m_eventQueue.pop();
        }
    }

    for(auto it : m_senders) {
        std::lock_guard<std::mutex> locker(lockMutex(it.sender));
        for(auto rcv = it.sender->m_recievers.begin(); rcv != it.sender->m_recievers.end(); ) {
            if(*rcv == it) {
                rcv = it.sender->m_recievers.erase(rcv);
            } else {
                rcv++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> locker(lockMutex(this));
        m_senders.clear();
    }

    for(auto it : m_recievers) {
        std::lock_guard<std::mutex> locker(lockMutex(it.receiver));
        for(auto snd = it.receiver->m_senders.begin(); snd != it.receiver->m_senders.end(); ) {
            if(*snd == it) {
                snd = it.receiver->m_senders.erase(snd);
            } else {
                snd++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> locker(lockMutex(this));
        m_recievers.clear();
    }

    for(const auto &it : m_children) {
        Object *c = it;
        if(c) {
            c->m_parent = nullptr;
            delete c;
        }
    }
    m_children.clear();

    if(m_parent) {
        m_parent->removeChild(this);
    }
}
/*!
    Returns new instance of Object class.
    This method is used in MetaObject system.

    \sa MetaObject
*/
Object *Object::construct() {
    PROFILE_FUNCTION();
    return new Object();
}
/*!
    Returns MetaObject and can be invoke without object of current class.
    This method is used in MetaObject system.

    \sa MetaObject
*/
const MetaObject *Object::metaClass() {
    PROFILE_FUNCTION();
    static const MetaObject staticMetaData("Object", nullptr, &construct, Object::methods(), nullptr, nullptr);
    return &staticMetaData;
}
/*!
    Returns ponter MetaObject of this object.
    This method is used in MetaObject system.

    \sa MetaObject
*/
const MetaObject *Object::metaObject() const {
    PROFILE_FUNCTION();
    return Object::metaClass();
}
/*!
    Clones this object.
    Returns pointer to clone object.

    When you clone the Object or subclasses of it, all child objects also will be cloned.
    By default the \a parent for the new object will be nullptr.
    This clone will not have the unique name so you will need to set it manualy if required.

    \warning Connections will NOT be transferred and the developer must create them manually.

    \sa connect()
*/
Object *Object::clone(Object *parent) {
    PROFILE_FUNCTION();

    ObjectList list;
    enumObjects(this, list);

    std::list<std::pair<const Object *, Object *>> array;

    for(auto it : list) {
        const MetaObject *meta = it->metaObject();
        Object *result = meta->createInstance();
        result->m_uuid = ObjectSystem::generateUUID();

        result->m_cloned = it->m_cloned;
        if(result->m_cloned == 0) {
            result->m_cloned = it->m_uuid;
        }

        Object *p = parent;
        for(auto item : array) {
            uint32_t uuid = (it->parent()->clonedFrom() != 0) ? it->parent()->clonedFrom() : it->parent()->uuid();
            if(item.second->clonedFrom() == uuid) {
                p = item.second;
                break;
            }
        }
        result->setParent(p);
        result->setSystem(it->m_system);
        result->setName(it->name());

        array.push_back(std::pair<const Object *, Object *>(it, result));
    }

    for(auto it : array) {
        const MetaObject *meta = it.first->metaObject();

        for(int i = 0; i < meta->propertyCount(); i++) {
            MetaProperty rp = meta->property(i);
            MetaProperty lp = it.second->metaObject()->property(i);
            Variant data = rp.read(it.first);
            if(rp.type().flags() & MetaType::BASE_OBJECT) {
                Object *ro = *(reinterpret_cast<Object **>(data.data()));

                for(auto item : array) {
                    if(item.first == ro) {
                        ro = item.second;
                        break;
                    }
                }

                data = Variant(data.userType(), &ro);
            }
            lp.write(it.second, data);
        }
    }

    return array.front().second;
}
/*!
    Returns the UUID of cloned object.
*/
uint32_t Object::clonedFrom() const {
    PROFILE_FUNCTION();
    return m_cloned;
}
/*!
    Returns a pointer to the parent object.
*/
Object *Object::parent() const {
    PROFILE_FUNCTION();
    return m_parent;
}
/*!
    Returns name of the object.
*/
std::string Object::name() const {
    PROFILE_FUNCTION();
    return m_name;
}
/*!
    Returns unique ID of the object.
*/
uint32_t Object::uuid() const {
    PROFILE_FUNCTION();
    return m_uuid;
}
/*!
    Returns class name the object.
*/
std::string Object::typeName() const {
    PROFILE_FUNCTION();
    return metaObject()->name();
}
/*!
    Creates connection beteen the \a signal of the \a sender and the \a method of the \a receiver.
    Returns true if successful; otherwise returns false.

    You must use the _SIGNAL() and _SLOT() macros when specifying \a signal and the \a method.
    \note The _SIGNAL() and _SLOT() must not contain any parameter values only parameter types.
    \code
        class MyObject : public Object {
            A_OVERRIDE(MyObject, Object, Core)

            A_METHODS(
                A_SLOT(onSignal),
                A_SIGNAL(signal)
            )
        public:
            void signal(bool value);

            void onSignal(bool value) {
                // Do some actions here
                ...
            }
        };
        ...
        MyObject obj1;
        MyObject obj2;

        Object::connect(&obj1, _SIGNAL(signal(bool)), &obj2, _SLOT(onSignal(bool)));
    \endcode
    \note Mehod signal in MyObject class may not have the implementation. It used only in description purposes in A_SIGNAL(signal) macros.

    Signal can also be conected to another signal.
    \code
        MyObject obj1;
        MyObject obj2;

        Object::connect(&obj1, _SIGNAL(signal(bool)), &obj2, _SIGNAL(signal(bool)));
    \endcode
*/
bool Object::connect(Object *sender, const char *signal, Object *receiver, const char *method) {
    PROFILE_FUNCTION();
    if(sender && receiver) {
        int32_t snd = sender->metaObject()->indexOfSignal(&signal[1]);

        int32_t rcv;
        MetaMethod::MethodType right = MetaMethod::MethodType(method[0] - 0x30);
        if(right == MetaMethod::Slot) {
            rcv = receiver->metaObject()->indexOfSlot(&method[1]);
        } else {
            rcv = receiver->metaObject()->indexOfSignal(&method[1]);
        }

        if(snd > -1 && rcv > -1) {
            Link link;

            link.sender = sender;
            link.signal = snd;
            link.receiver = receiver;
            link.method = rcv;

            if(!sender->isLinkExist(link)) {
                {
                    std::lock_guard<std::mutex> locker(lockMutex(sender));
                    sender->m_recievers.push_back(link);
                }
                {
                    std::lock_guard<std::mutex> locker(lockMutex(receiver));
                    receiver->m_senders.push_back(link);
                }
                return true;
            }
        }
    }
    return false;
}
/*!
    Disconnects \a signal in object \a sender from \a method in object \a receiver.

    A connection is removed when either of the objects are destroyed.

    disconnect() can be used in three ways:

    Disconnect everything from a specific sender...
    \code
        Object::disconnect(&obj1, 0, 0, 0);
    \endcode
    Disconnect everything connected to a specific signal...
    \code
        Object::disconnect(&obj1, _SIGNAL(signal(bool)), 0, 0);
    \endcode
    Disconnect all connections from the receiver...
    \code
        Object::disconnect(&obj1, 0, &obj3, 0);
    \endcode

    \sa connect()
*/
void Object::disconnect(Object *sender, const char *signal, Object *receiver, const char *method) {
    PROFILE_FUNCTION();
    if(sender) {
        std::lock_guard<std::mutex> slocker(lockMutex(sender));
        for(auto snd = sender->m_recievers.begin(); snd != sender->m_recievers.end(); ) {
            Link data = *snd;
            if(data.sender == sender) {
                if(signal == nullptr || data.signal == sender->metaObject()->indexOfMethod(&signal[1])) {
                    if(receiver == nullptr || data.receiver == receiver) {
                        if(method == nullptr || (receiver && data.method == receiver->metaObject()->indexOfMethod(&method[1]))) {
                            if(data.receiver != sender) {
                                lockMutex(data.receiver).lock();
                            }
                            for(auto rcv = data.receiver->m_senders.begin(); rcv != data.receiver->m_senders.end(); ) {
                                if(*rcv == data) {
                                    rcv = data.receiver->m_senders.erase(rcv);
                                } else {
                                    rcv++;
                                }
                            }
                            if(data.receiver != sender) {
                                lockMutex(data.receiver).unlock();
                            }

                            snd = sender->m_recievers.erase(snd);
                            continue;
                        }
                    }
                }
            }
            snd++;
        }
    }
}
/*!
    Marks this object to be deleted.
    This object will be deleted when event loop will call processEvents() method for this object.
*/
void Object::deleteLater() {
    PROFILE_FUNCTION();
    postEvent(new Event(Event::Destroy));
}
/*!
    Returns list of child objects for this object.
*/
const Object::ObjectList &Object::getChildren() const {
    PROFILE_FUNCTION();
    return m_children;
}
/*!
    Returns list of links to receivers objects for this object.
*/
const Object::LinkList &Object::getReceivers() const {
    PROFILE_FUNCTION();
    return m_recievers;
}

std::vector<std::string> split(const std::string &s, char seperator) {
    std::vector<std::string>container;
    std::istringstream f(s);
    std::istringstream &stream = f;
    std::string out;
    while(std::getline(stream, out, seperator)) {
        container.push_back(out);
    }
    return container;
}

/*!
    Returns an object located along the \a path.

    \code
        Object obj1;
        Object obj2;

        obj1.setName("MainObject");
        obj2.setName("TestComponent2");
        obj2.setParent(&obj1);

        // result will contain pointer to obj2
        Object *result = obj1.find("/MainObject/TestComponent2");
    \endcode

    Returns nullptr if no such object.

    \sa findChild()
*/
Object *Object::find(const std::string &path) {
    PROFILE_FUNCTION();

    Object *root = this;

    auto list = split(path, '/');

    bool found = false;

    for(auto name : list) {
        if(name.empty()) {
            while(root->m_parent != nullptr) {
                root = root->m_parent;
            }
        } else {
            for(const auto &it : root->m_children) {
                if(it->m_name == name) {
                    root = it;
                    found = true;
                    break;
                }
            }
        }
    }

    if(found) {
        return root;
    }

    return nullptr;
}
/*!
    Makes the object a child of \a parent at given \a position.
    \note Please ignore the \a force flag it will be provided by the default.

    \sa parent()
*/
void Object::setParent(Object *parent, int32_t position, bool force) {
    PROFILE_FUNCTION();
    A_UNUSED(force);

    if(parent == this) {
        return;
    }

    if(m_parent) {
        m_parent->removeChild(this);
    }
    if(parent) {
        parent->addChild(this, position);
    }
    m_parent = parent;
}
/*!
    Set object name by provided \a name.

    \sa metaObject()
*/
void Object::setName(const std::string &name) {
    PROFILE_FUNCTION();
    if(!name.empty()) {
        m_name = name;
    }
}
/*!
    Pushes a \a child object to the internal list of children at given \a position.
*/
void Object::addChild(Object *child, int32_t position) {
    PROFILE_FUNCTION();
    if(child) {
        if(position == -1 || m_children.size() < position) {
            m_children.push_back(child);
        } else {
            m_children.insert(next(m_children.begin(), position), child);
        }
    }
}
/*!
    Removes a \a child object from the internal list of children.
*/
void Object::removeChild(Object *child) {
    PROFILE_FUNCTION();
    auto it = m_children.begin();
    while(it != m_children.end()) {
        if(*it == child) {
            m_children.erase(it);
            return;
        }
        it++;
    }
}
/*!
    Send specific \a signal with \a args for all connected receivers.

    For now it places signal directly to receivers queues.
    In case of another signal connected as method this signal will be emitted immediately.

    \note Receiver should be in event loop to process incoming message.

    \sa connect()
*/
void Object::emitSignal(const char *signal, const Variant &args) {
    PROFILE_FUNCTION();
    int32_t index = metaObject()->indexOfSignal(&signal[1]);
    std::lock_guard<std::mutex> locker(lockMutex(this));
    for(auto &it : m_recievers) {
        Link *link = &(it);
        if(link->signal == index) {
            const MetaMethod &method = link->receiver->metaObject()->method(link->method);
            if(method.isValid()) {
                if(method.type() == MetaMethod::Signal) {
                    link->receiver->emitSignal(std::string(char(method.type() + 0x30) + method.signature()).c_str(), args);
                } else {
                    if(m_system && link->receiver->m_system &&
                       !m_system->compareTreads(link->receiver->m_system)) { // Queued Connection

                        link->receiver->postEvent(new MethodCallEvent(link->method, link->sender, args));
                    } else { // Direct call
                        MethodCallEvent e(link->method, link->sender, args);
                        link->receiver->methodCallEvent(&e);
                    }
                    link = nullptr;
                }
            }
        }
    }
}
/*!
    Place event to internal \a event queue to be processed in event loop.
*/
void Object::postEvent(Event *event) {
    PROFILE_FUNCTION();
    std::lock_guard<std::mutex> locker(lockMutex(this));
    m_eventQueue.push(event);
}
/*!
    \internal
*/
void Object::processEvents() {
    PROFILE_FUNCTION();

    while(!m_eventQueue.empty()) {
        Event *e = nullptr;
        {
            std::lock_guard<std::mutex> locker(lockMutex(this));
            e = m_eventQueue.front();
            m_eventQueue.pop();
        }

        switch (e->type()) {
            case Event::MethodCall: {
                methodCallEvent(reinterpret_cast<MethodCallEvent *>(e));
            } break;
            case Event::Destroy: {
                if(m_system) {
                    m_system->suspendObject(this);
                }

                delete e;
                delete this;
                return;
            }
            default: {
                event(e);
            } break;
        }
        delete e;
    }
}
/*!
    Abstract event handler.
    Developers should reimplement this method to handle events manually.
    Returns true in case of \a event was handled otherwise return false.
*/
bool Object::event(Event *event) {
    PROFILE_FUNCTION();
    A_UNUSED(event);
    return false;
}
/*!
    This method allows to DESERIALIZE \a data of object like properties, connections and user data.
*/
void Object::loadData(const VariantList &data) {
    A_UNUSED(data);
}
/*!
    This method allows to DESERIALIZE \a data.
    It can be used to DESERIALIZE some specific data like prefabs.
*/
void Object::loadObjectData(const VariantMap &data) {
    A_UNUSED(data);
}
/*!
    This method allows to DESERIALIZE \a data which not present as A_PROPERTY() in object.
*/
void Object::loadUserData(const VariantMap &data) {
    A_UNUSED(data);
}
/*!
    This method allows to SERIALIZE all object data like properties connections and user data.
    Returns serialized data as VariantList.
*/
VariantList Object::saveData() const {
    return serializeData(metaObject());
}
/*!
    This method allows to SERIALIZE data which not present as A_PROPERTY() in object.
    Returns serialized data as VariantMap.
*/
VariantMap Object::saveUserData() const {
    return VariantMap();
}
/*!
    Specify an additional \a type for the object.
    \note Most of the time this method does nothing.
*/
void Object::setType(const std::string &type) {
    A_UNUSED(type);
}
/*!
    Returns true if the object can be serialized; otherwise returns false.
*/
bool Object::isSerializable() const {
    return true;
}
/*!
    Returns the value of the object's property by \a name.

    If property not found returns invalid Variant.
    Information of all properties which provided by this object can be found in MetaObject.

    \sa setProperty(), metaObject(), Variant::isValid()
*/
Variant Object::property(const char *name) const {
    PROFILE_FUNCTION();
    const MetaObject *meta = metaObject();
    int index = meta->indexOfProperty(name);
    if(index < 0) { // Check dynamic property
        auto it = std::find(m_dynamicPropertyNames.begin(), m_dynamicPropertyNames.end(), name);
        if(it == m_dynamicPropertyNames.end()) {
            return Variant();
        }

        size_t index = std::distance(m_dynamicPropertyNames.begin(), it);
        return *std::next(m_dynamicPropertyValues.begin(), index);
    }

    return meta->property(index).read(this);
}
/*!
    Sets the property with \a name to \a value.

    If property not found do nothing.
    Property must be defined as A_PROPERTY().
    Information of all properties which provided by this object can be found in MetaObject.

    \sa property(), metaObject(), Variant::isValid()
*/
void Object::setProperty(const char *name, const Variant &value) {
    PROFILE_FUNCTION();
    const MetaObject *meta = metaObject();
    int index = meta->indexOfProperty(name);
    if(index < 0) {
        auto nameIterator = std::find(m_dynamicPropertyNames.begin(), m_dynamicPropertyNames.end(), name);
        if(nameIterator != m_dynamicPropertyNames.end()) {
            index = std::distance(m_dynamicPropertyNames.begin(), nameIterator);
        }

        if(!value.isValid() && index > -1) { // Remove dynamic property if exists
            auto valueIterator = std::next(m_dynamicPropertyValues.begin(), index);
            m_dynamicPropertyNames.erase(nameIterator);
            m_dynamicPropertyValues.erase(valueIterator);
        } else { // Set a new value
            if(index < 0) {
                m_dynamicPropertyNames.push_back(name);
                m_dynamicPropertyValues.push_back(value);
            } else {
                *std::next(m_dynamicPropertyValues.begin(), index) = value;
            }
        }

        return;
    }

    meta->property(index).write(this, value);
}
/*!
    Returns the names of all properties that were dynamically added to the object using setProperty()
*/
const std::list<std::string> Object::dynamicPropertyNames() const {
    return m_dynamicPropertyNames;
}
/*!
    Returns object which sent signal.
    \note This method returns a valid object only in receiver slot otherwise it's return nullptr
*/
Object *Object::sender() const {
    PROFILE_FUNCTION();
    return m_currentSender;
}
/*!
    Returns System which handles this object.
*/
ObjectSystem *Object::system() const {
    PROFILE_FUNCTION()
    return m_system;
}
/*!
    Method call \a event handler. Can be reimplemented to support different logic.
*/
void Object::methodCallEvent(MethodCallEvent *event) {
    m_currentSender = event->sender();
    Variant result;
    if(event->args()->isValid()) {
        metaObject()->method(event->method()).invoke(this, result, 1, event->args());
    } else {
        metaObject()->method(event->method()).invoke(this, result, 0, nullptr);
    }
    m_currentSender = nullptr;
}
/*!
    \internal
*/
void Object::clearCloneRef() {
    m_cloned = 0;
}

void Object::setUUID(uint32_t id) {
    PROFILE_FUNCTION();
    m_uuid = id;
}
/*!
    \internal
*/
void Object::setSystem(ObjectSystem *system) {
    PROFILE_FUNCTION();
    m_system = system;
    m_system->addObject(this);
}
/*!
    \internal
*/
VariantList Object::serializeData(const MetaObject *meta) const {
    PROFILE_FUNCTION()

    VariantList result;

    result.push_back(meta->name());
    result.push_back(static_cast<int32_t>(uuid()));
    Object *p = parent();
    result.push_back(static_cast<int32_t>(((p) ? p->uuid() : 0)));
    result.push_back(name());

    // Save base properties
    VariantMap properties;
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        if(property.isValid()) {
            Variant v = property.read(this);
            uint32_t type = v.userType();
            if(type < MetaType::USERTYPE && type != MetaType::VARIANTLIST && type != MetaType::VARIANTMAP) {
                properties[property.name()] = v;
            }
        }
    }

    // Save dynamic properties
    VariantList dynamic;
    auto dynamicNames = m_dynamicPropertyNames.begin();
    auto dynamicValues = m_dynamicPropertyValues.begin();
    for(int i = 0; i < m_dynamicPropertyNames.size(); i++) {
        VariantList pair;
        pair.push_back(*dynamicNames);
        pair.push_back(*dynamicValues);

        dynamic.push_back(pair);

        ++dynamicNames;
        ++dynamicValues;
    }

    // Save links
    VariantList links;
    for(const auto &l : getReceivers()) {
        VariantList link;

        Object *receiver = l.receiver;

        link.push_back(static_cast<int32_t>(uuid()));
        MetaMethod method = meta->method(l.signal);
        link.push_back(Variant(char(method.type() + 0x30) + method.signature()));

        link.push_back(static_cast<int32_t>(receiver->uuid()));
        method = receiver->metaObject()->method(l.method);
        link.push_back(Variant(char(method.type() + 0x30) + method.signature()));

        links.push_back(link);
    }
    result.push_back(properties);
    result.push_back(links);
    result.push_back(saveUserData());
    if(!dynamic.empty()) {
        result.push_back(dynamic);
    }

    return result;
}
/*!
    \internal
*/
bool Object::isLinkExist(const Object::Link &link) const {
    PROFILE_FUNCTION();
    for(const auto &it : m_recievers) {
        if(it == link) {
            return true;
        }
    }
    return false;
}
/*!
    \internal
*/
void Object::enumObjects(Object *object, Object::ObjectList &list) {
    list.push_back(object);
    PROFILE_FUNCTION();
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}
