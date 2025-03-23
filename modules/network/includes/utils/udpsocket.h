#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "socket.h"

class NETWORK_EXPORT UdpSocket : public Socket {
public:
    bool bind(const NetworkAddress &address) override;

    uint64_t readDatagram(int8_t *data, uint64_t maxSize, NetworkAddress *address);

    uint64_t writeDatagram(int8_t *data, uint64_t size, const NetworkAddress &address);

};

#endif // UDPSOCKET_H
