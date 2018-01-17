#include "core/aobjectsystem.h"
#include "core/auri.h"

#include <mutex>

inline bool operator==(const AObject::Link &left, const AObject::Link &right) {
    bool result = true;
    result &= (left.sender      == right.sender);
    result &= (left.receiver    == right.receiver);
    result &= (left.signal      == right.signal);
    result &= (left.method      == right.method);
    return result;
}

class AObjectPrivate {
public:
    AObjectPrivate() :
        m_bEnable(true),
        m_pParent(nullptr),
        m_pCurrentSender(nullptr),
        m_UUID(0) {

    }

    bool isLinkExist(const AObject::Link &link) const {
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
    AObject                        *m_pParent;
    /// Object name
    string                          m_sName;

    AObject::ObjectList             m_mChildren;
    AObject::LinkList               m_lRecievers;
    AObject::LinkList               m_lSenders;

    AObject                        *m_pCurrentSender;

    typedef queue<AEvent *>         EventQueue;
    EventQueue                      m_EventQueue;

    uint32_t                        m_UUID;

    mutex                           m_Mutex;

};

AObject::AObject() :
        p_ptr(new AObjectPrivate) {
    PROFILE_FUNCTION()

    onCreated();
}

AObject::~AObject() {
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
        AObject *c  = it;
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

AObject *AObject::construct() {
    PROFILE_FUNCTION()
    return new AObject();
}

const AMetaObject *AObject::metaClass() {
    PROFILE_FUNCTION()
    static const AMetaObject staticMetaData("AObject", nullptr, &construct, nullptr, nullptr);
    return &staticMetaData;
}

const AMetaObject *AObject::metaObject() const {
    PROFILE_FUNCTION()
    return AObject::metaClass();
}

AObject *AObject::clone() {
    PROFILE_FUNCTION()
    const AMetaObject *meta = metaObject();
    AObject *result = meta->createInstance();
    result->p_ptr->m_UUID  = AObjectSystem::instance()->nextID();
    int count  = meta->propertyCount();
    for(int i = 0; i < count; i++) {
        AMetaProperty lp    = result->metaObject()->property(i);
        AMetaProperty rp    = meta->property(i);
        lp.write(result, rp.read(this));
    }
    for(auto it : getChildren()) {
        AObject *clone  = it->clone();
        clone->setName(it->name());
        clone->setParent(result);
    }
    for(auto it : p_ptr->m_lSenders) {
        AMetaMethod signal  = it.sender->metaObject()->method(it.signal);
        AMetaMethod method  = result->metaObject()->method(it.method);
        connect(it.sender, (to_string(1) + signal.signature()).c_str(),
                result, (to_string((method.type() == AMetaMethod::Signal) ? 1 : 2) + method.signature()).c_str());
    }
    for(auto it : getReceivers()) {
        AMetaMethod signal  = result->metaObject()->method(it.signal);
        AMetaMethod method  = it.receiver->metaObject()->method(it.method);
        connect(result, (to_string(1) + signal.signature()).c_str(),
                it.receiver, (to_string((method.type() == AMetaMethod::Signal) ? 1 : 2) + method.signature()).c_str());
    }
    return result;
}

AObject *AObject::parent() const {
    PROFILE_FUNCTION()
    return p_ptr->m_pParent;
}

string AObject::name() const {
    PROFILE_FUNCTION()
    return p_ptr->m_sName;
}

uint32_t AObject::uuid() const {
    PROFILE_FUNCTION()
    return p_ptr->m_UUID;
}

string AObject::typeName() const {
    PROFILE_FUNCTION()
    return metaObject()->name();
}

void AObject::connect(AObject *sender, const char *signal, AObject *receiver, const char *method) {
    PROFILE_FUNCTION()
    if(sender && receiver) {
        int32_t snd = sender->metaObject()->indexOfSignal(&signal[1]);

        int32_t rcv;
        AMetaMethod::MethodType right   = AMetaMethod::MethodType(method[0] - 0x30);
        if(right == AMetaMethod::Slot) {
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

void AObject::disconnect(AObject *sender, const char *signal, AObject *receiver, const char *method) {
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

void AObject::deleteLater() {
    PROFILE_FUNCTION()
    postEvent(new AEvent(AEvent::Delete));
}

const AObject::ObjectList &AObject::getChildren() const {
    PROFILE_FUNCTION()
    return p_ptr->m_mChildren;
}

const AObject::LinkList &AObject::getReceivers() const {
    PROFILE_FUNCTION()
    return p_ptr->m_lRecievers;
}

AObject *AObject::find(const string &path) {
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
            AObject *o  = it->find(path.substr(index + 1));
            if(o) {
                return o;
            }
        }
    } else if(path.substr(start, index) == p_ptr->m_sName) {
        return this;
    }

    return nullptr;
}

void AObject::setParent(AObject *parent) {
    PROFILE_FUNCTION()
    if(p_ptr->m_pParent) {
        p_ptr->m_pParent->removeChild(this);
    }
    if(parent) {
        parent->addChild(this);
    }
    p_ptr->m_pParent    = parent;
}

void AObject::setName(const string &value) {
    PROFILE_FUNCTION()
    if(!value.empty()) {
        p_ptr->m_sName = value;
        // \todo Notify receivers
    }
}

void AObject::addChild(AObject *value) {
    PROFILE_FUNCTION()
    if(value) {
        p_ptr->m_mChildren.push_back(value);
    }
}

void AObject::removeChild(AObject *value) {
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

bool AObject::isEnable() const {
    PROFILE_FUNCTION()
    return p_ptr->m_bEnable;
}

void AObject::emitSignal(const char *signal, const AVariant &args) {
    PROFILE_FUNCTION()
    int32_t index   = metaObject()->indexOfSignal(&signal[1]);
    for(auto &it : p_ptr->m_lRecievers) {
        Link *link  = &(it);
        if(link->signal == index) {
            const AMetaMethod &method   = link->receiver->metaObject()->method(link->method);
            if(method.type() == AMetaMethod::Signal) {
                link->receiver->emitSignal(string(char(method.type() + 0x30) + method.signature()).c_str(), args);
            } else {
                // Queued Connection
                link->receiver->postEvent(new AMethodCallEvent(link->method, link->sender, args));
            }
        }
    }
}

bool AObject::postEvent(AEvent *e) {
    PROFILE_FUNCTION()
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_EventQueue.push(e);

    return true;
}

void AObject::processEvents() {
    PROFILE_FUNCTION()
    while(!p_ptr->m_EventQueue.empty()) {
        unique_lock<mutex> locker(p_ptr->m_Mutex);
        AEvent *e   = p_ptr->m_EventQueue.front();
        switch (e->type()) {
            case AEvent::MethodCall: {
                AMethodCallEvent *call  = reinterpret_cast<AMethodCallEvent *>(e);
                p_ptr->m_pCurrentSender = call->sender();
                AVariant result;
                metaObject()->method(call->method()).invoke(this, result, 1, call->args());
                p_ptr->m_pCurrentSender = nullptr;
            } break;
            case AEvent::Delete: {
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

bool AObject::event(AEvent *) {
    PROFILE_FUNCTION()
    return false;
}

void AObject::setEnable(bool state) {
    PROFILE_FUNCTION()
    p_ptr->m_bEnable   = state;
}

void AObject::loadUserData(const AVariantMap &) {

}

AVariantMap AObject::saveUserData() const {
    return AVariantMap();
}

AVariant AObject::property(const char *name) const {
    PROFILE_FUNCTION()
    const AMetaObject *meta = metaObject();
    int index   = meta->indexOfProperty(name);
    if(index > -1) {
        return meta->property(index).read(this);
    }
    return AVariant();
}

void AObject::setProperty(const char *name, const AVariant &value) {
    PROFILE_FUNCTION()
    const AMetaObject *meta = metaObject();
    int index   = meta->indexOfProperty(name);
    if(index > -1) {
        meta->property(index).write(this, value);
    }
}

void AObject::onCreated() {
    PROFILE_FUNCTION()

}

AObject *AObject::sender() const {
    PROFILE_FUNCTION()
    return p_ptr->m_pCurrentSender;
}

void AObject::setUUID(uint32_t id) {
    PROFILE_FUNCTION()
    p_ptr->m_UUID   = id;
}

AObject &AObject::operator=(AObject &right) {
    PROFILE_FUNCTION()
    return *new AObject(right);
}

AObject::AObject(const AObject &) {
    PROFILE_FUNCTION()
}
