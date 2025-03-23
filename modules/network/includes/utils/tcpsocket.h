#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "socket.h"

typedef struct ssl_st SSL;

class NETWORK_EXPORT TcpSocket : public Socket {
public:
    TcpSocket(bool ssl);

    bool connect(const NetworkAddress &address);

    void disconnect();

    uint64_t read(ByteArray &data);

    uint64_t write(const ByteArray &data);

private:
    SSL *m_ssl;

};

#endif // TCPSOCKET_H
