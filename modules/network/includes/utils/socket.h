#ifndef SOCKET_H
#define SOCKET_H

// platform detection

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MAC
#else
#define PLATFORM_LINUX
#endif

#ifdef PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <fcntl.h>
#endif

#include <networkaddress.h>

class NETWORK_EXPORT Socket {
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
