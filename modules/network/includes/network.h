#ifndef NETWORK_H
#define NETWORK_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef NETWORK_LIBRARY
        #define NETWORK_EXPORT __declspec(dllexport)
    #else
        #define NETWORK_EXPORT __declspec(dllimport)
    #endif
#else
    #define NETWORK_EXPORT
#endif

class NetworkSystem;

class Network : public Module {
public:
    Network(Engine *engine);
    ~Network();

    const char *metaInfo() const override;

};

#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // NETWORK_H
