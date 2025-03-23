#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "socket.h"

class NETWORK_EXPORT UdpSocket : public Socket {
public:
    bool bind(const NetworkAddress &address) override;

    uint64_t read(ByteArray &data, NetworkAddress *address);

    uint64_t write(const ByteArray &data, const NetworkAddress &address);

};

#endif // UDPSOCKET_H
