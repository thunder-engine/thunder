#include "core/objectsystem.h"
#include "core/uri.h"

#include <mutex>

inline bool operator==(const Object::Link &left, const Object::Link &right) {
    bool result = true;
    result &= (left.sender      == right.sender);
    result &= (left.receiver    == right.receiver);
    result &= (left.signal      == right.signal);
    result &= (left.method      == right.method);
    return result;
}

class ObjectPrivate {
public:
    ObjectPrivate() :
        m_bEnable(true),
        m_pParent(nullptr),
        m_pCurrentSender(nullptr),
        m_UUID(0) {

    }

    bool isLinkExist(const Object::Link &link) const {
        PROFILE_FUNCTION()
        for(const auto &it : m_lRecievers) {
            if(it == link) {
                return true;
            }
        }
        return false;
    }

    /// Enable object flag
    bool                            m_bEnable;
    /// Parent object
    Object                        *m_pParent;
    /// Object name
    string                          m_sName;

    Object::ObjectList             m_mChildren;
    Object::LinkList               m_lRecievers;
    Object::LinkList               m_lSenders;

    Object                        *m_pCurrentSender;

    typedef queue<Event *>         EventQueue;
    EventQueue                      m_EventQueue;

    uint32_t                        m_UUID;

    mutex                           m_Mutex;

};

Object::Object() :
        p_ptr(new ObjectPrivate) {
    PROFILE_FUNCTION()

    onCreated();
}

Object::~Object() {
    PROFILE_FUNCTION()
    {
        unique_lock<mutex> locker(p_ptr->m_Mutex);
        while(!p_ptr->m_EventQueue.empty()) {
            delete p_ptr->m_EventQueue.front();
            p_ptr->m_EventQueue.pop();
        }
    }

    while(!p_ptr->m_lSenders.empty()) {
        disconnect(p_ptr->m_lSenders.front().sender, 0, this, 0);
    }
    disconnect(this, 0, 0, 0);

    for(const auto &it : p_ptr->m_mChildren) {
        Object *c  = it;
        if(c) {
            c->p_ptr->m_pParent    = 0;
            delete c;
        }
    }
    p_ptr->m_mChildren.clear();

    if(p_ptr->m_pParent) {
        p_ptr->m_pParent->removeChild(this);
    }
}

Object *Object::construct() {
    PROFILE_FUNCTION()
    return new Object();
}

const MetaObject *Object::metaClass() {
    PROFILE_FUNCTION()
    static const MetaObject staticMetaData("Object", nullptr, &construct, nullptr, nullptr);
    return &staticMetaData;
}

const MetaObject *Object::metaObject() const {
    PROFILE_FUNCTION()
    return Object::metaClass();
}

Object *Object::clone() {
    PROFILE_FUNCTION()
    const MetaObject *meta  = metaObject();
    Object *result = meta->createInstance();
    result->p_ptr->m_UUID   = ObjectSystem::instance()->nextID();
    int count  = meta->propertyCount();
    for(int i = 0; i < count; i++) {
        MetaProperty lp = result->metaObject()->property(i);
        MetaProperty rp = meta->property(i);
        lp.write(result, rp.read(this));
    }
    for(auto it : getChildren()) {
        Object *clone  = it->clone();
        clone->setName(it->name());
        clone->setParent(result);
    }
    for(auto it : p_ptr->m_lSenders) {
        MetaMethod signal  = it.sender->metaObject()->method(it.signal);
        MetaMethod method  = result->metaObject()->method(it.method);
        connect(it.sender, (to_string(1) + signal.signature()).c_str(),
                result, (to_string((method.type() == MetaMethod::Signal) ? 1 : 2) + method.signature()).c_str());
    }
    for(auto it : getReceivers()) {
        MetaMethod signal  = result->metaObject()->method(it.signal);
        MetaMethod method  = it.receiver->metaObject()->method(it.method);
        connect(result, (to_string(1) + signal.signature()).c_str(),
                it.receiver, (to_string((method.type() == MetaMethod::Signal) ? 1 : 2) + method.signature()).c_str());
    }
    return result;
}

Object *Object::parent() const {
    PROFILE_FUNCTION()
    return p_ptr->m_pParent;
}

string Object::name() const {
    PROFILE_FUNCTION()
    return p_ptr->m_sName;
}

uint32_t Object::uuid() const {
    PROFILE_FUNCTION()
    return p_ptr->m_UUID;
}

string Object::typeName() const {
    PROFILE_FUNCTION()
    return metaObject()->name();
}

void Object::connect(Object *sender, const char *signal, Object *receiver, const char *method) {
    PROFILE_FUNCTION()
    if(sender && receiver) {
        int32_t snd = sender->metaObject()->indexOfSignal(&signal[1]);

        int32_t rcv;
        MetaMethod::MethodType right   = MetaMethod::MethodType(method[0] - 0x30);
        if(right == MetaMethod::Slot) {
            rcv = receiver->metaObject()->indexOfSlot(&method[1]);
        } else {
            rcv = receiver->metaObject()->indexOfSignal(&method[1]);
        }

        if(snd > -1 && rcv > -1) {
            Link link;

            link.sender     = sender;
            link.signal     = snd;
            link.receiver   = receiver;
            link.method     = rcv;

            if(!sender->p_ptr->isLinkExist(link)) {
                {
                    unique_lock<mutex> locker(sender->p_ptr->m_Mutex);
                    sender->p_ptr->m_lRecievers.push_back(link);
                }
                {
                    unique_lock<mutex> locker(receiver->p_ptr->m_Mutex);
                    receiver->p_ptr->m_lSenders.push_back(link);
                }
            }
        }
    }
}

void Object::disconnect(Object *sender, const char *signal, Object *receiver, const char *method) {
    PROFILE_FUNCTION()
    if(sender && !sender->p_ptr->m_lRecievers.empty()) {
        for(auto snd = sender->p_ptr->m_lRecievers.begin(); snd != sender->p_ptr->m_lRecievers.end(); snd) {
            Link *data = &(*snd);

            if(data->sender == sender) {
                if(signal == nullptr || data->signal == sender->metaObject()->indexOfMethod(&signal[1])) {
                    if(receiver == nullptr || data->receiver == receiver) {
                        if(method == nullptr || (receiver && data->method == receiver->metaObject()->indexOfMethod(&method[1]))) {

                            for(auto rcv = data->receiver->p_ptr->m_lSenders.begin(); rcv != data->receiver->p_ptr->m_lSenders.end(); rcv) {
                                if(*rcv == *data) {
                                    unique_lock<mutex> locker(data->receiver->p_ptr->m_Mutex);
                                    rcv = data->receiver->p_ptr->m_lSenders.erase(rcv);
                                } else {
                                    rcv++;
                                }
                            }
                            unique_lock<mutex> locker(sender->p_ptr->m_Mutex);
                            snd = sender->p_ptr->m_lRecievers.erase(snd);

                            continue;
                        }
                    }
                }
            }
            snd++;
        }
    }
}

void Object::deleteLater() {
    PROFILE_FUNCTION()
    postEvent(new Event(Event::Delete));
}

const Object::ObjectList &Object::getChildren() const {
    PROFILE_FUNCTION()
    return p_ptr->m_mChildren;
}

const Object::LinkList &Object::getReceivers() const {
    PROFILE_FUNCTION()
    return p_ptr->m_lRecievers;
}

Object *Object::find(const string &path) {
    PROFILE_FUNCTION()
    if(p_ptr->m_pParent && path[0] == '/') {
        return p_ptr->m_pParent->find(path);
    }

    unsigned int start  = 0;
    if(path[0] == '/') {
        start   = 1;
    }
    int index  = path.find('/', 1);
    if(index > -1) {
        for(const auto &it : p_ptr->m_mChildren) {
            Object *o  = it->find(path.substr(index + 1));
            if(o) {
                return o;
            }
        }
    } else if(path.substr(start, index) == p_ptr->m_sName) {
        return this;
    }

    return nullptr;
}

void Object::setParent(Object *parent) {
    PROFILE_FUNCTION()
    if(p_ptr->m_pParent) {
        p_ptr->m_pParent->removeChild(this);
    }
    if(parent) {
        parent->addChild(this);
    }
    p_ptr->m_pParent    = parent;
}

void Object::setName(const string &value) {
    PROFILE_FUNCTION()
    if(!value.empty()) {
        p_ptr->m_sName = value;
        // \todo Notify receivers
    }
}

void Object::addChild(Object *value) {
    PROFILE_FUNCTION()
    if(value) {
        p_ptr->m_mChildren.push_back(value);
    }
}

void Object::removeChild(Object *value) {
    PROFILE_FUNCTION()
    auto it = p_ptr->m_mChildren.begin();
    while(it != p_ptr->m_mChildren.end()) {
        if(*it == value) {
            p_ptr->m_mChildren.erase(it);
            return;
        }
        it++;
    }
}

bool Object::isEnable() const {
    PROFILE_FUNCTION()
    return p_ptr->m_bEnable;
}

void Object::emitSignal(const char *signal, const Variant &args) {
    PROFILE_FUNCTION()
    int32_t index   = metaObject()->indexOfSignal(&signal[1]);
    for(auto &it : p_ptr->m_lRecievers) {
        Link *link  = &(it);
        if(link->signal == index) {
            const MetaMethod &method   = link->receiver->metaObject()->method(link->method);
            if(method.type() == MetaMethod::Signal) {
                link->receiver->emitSignal(string(char(method.type() + 0x30) + method.signature()).c_str(), args);
            } else {
                // Queued Connection
                link->receiver->postEvent(new MethodCallEvent(link->method, link->sender, args));
            }
        }
    }
}

bool Object::postEvent(Event *e) {
    PROFILE_FUNCTION()
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_EventQueue.push(e);

    return true;
}

void Object::processEvents() {
    PROFILE_FUNCTION()
    while(!p_ptr->m_EventQueue.empty()) {
        unique_lock<mutex> locker(p_ptr->m_Mutex);
        Event *e   = p_ptr->m_EventQueue.front();
        switch (e->type()) {
            case Event::MethodCall: {
                MethodCallEvent *call   = reinterpret_cast<MethodCallEvent *>(e);
                p_ptr->m_pCurrentSender = call->sender();
                Variant result;
                metaObject()->method(call->method()).invoke(this, result, 1, call->args());
                p_ptr->m_pCurrentSender = nullptr;
            } break;
            case Event::Delete: {
                locker.unlock();
                delete this;
                return;
            } break;
            default: {
                event(e);
            } break;
        }
        delete e;
        p_ptr->m_EventQueue.pop();
    }
}

bool Object::event(Event *) {
    PROFILE_FUNCTION()
    return false;
}

void Object::setEnable(bool state) {
    PROFILE_FUNCTION()
    p_ptr->m_bEnable   = state;
}

void Object::loadUserData(const VariantMap &) {

}

VariantMap Object::saveUserData() const {
    return VariantMap();
}

Variant Object::property(const char *name) const {
    PROFILE_FUNCTION()
    const MetaObject *meta  = metaObject();
    int index   = meta->indexOfProperty(name);
    if(index > -1) {
        return meta->property(index).read(this);
    }
    return Variant();
}

void Object::setProperty(const char *name, const Variant &value) {
    PROFILE_FUNCTION()
    const MetaObject *meta  = metaObject();
    int index   = meta->indexOfProperty(name);
    if(index > -1) {
        meta->property(index).write(this, value);
    }
}

void Object::onCreated() {
    PROFILE_FUNCTION()

}

Object *Object::sender() const {
    PROFILE_FUNCTION()
    return p_ptr->m_pCurrentSender;
}

void Object::setUUID(uint32_t id) {
    PROFILE_FUNCTION()
    p_ptr->m_UUID   = id;
}

Object &Object::operator=(Object &right) {
    PROFILE_FUNCTION()
    return *new Object(right);
}

Object::Object(const Object &) {
    PROFILE_FUNCTION()
}
