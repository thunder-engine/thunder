#include "network.h"

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new Network(engine);
}
#endif

static const char *meta = \
"{"
"   \"version\": \"1.0\","
"   \"module\": \"Network\","
"   \"description\": \"Network Module\","
"   \"author\": \"Evgeniy Prikazchikov\""
"}";

Network::Network(Engine *engine) :
        Module(engine) {

}

Network::~Network() {

}

const char *Network::metaInfo() const {
    return meta;
}
