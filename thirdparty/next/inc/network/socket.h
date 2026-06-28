#ifndef SOCKET_H
#define SOCKET_H

#include <networkaddress.h>

class NEXT_LIBRARY_EXPORT Socket {
public:
    Socket();

    virtual ~Socket();

    virtual bool bind(const NetworkAddress &address);

    void close();

    bool isValid() const;

    bool isDataAvailable() const;

protected:
    NetworkAddress m_localAddress;
    NetworkAddress m_peerAddress;

    int32_t m_socket;

};

#endif // SOCKET_H
