#include "socket.h"

#include "networkaddress.h"

Socket::Socket() :
        m_socket(0) {

#ifdef PLATFORM_WINDOWS
    static bool init = false;
    if(!init) {
        WSADATA wsaData;
        init = (WSAStartup(WINSOCK_VERSION, &wsaData) == 0);
    }
#endif
}

Socket::~Socket() {
    close();
}

bool Socket::bind(const NetworkAddress &address) {
    if(m_socket <= 0) {
        m_socket = 0;
        return false;
    }

    m_localAddress = address;

    // bind to port
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(m_localAddress.toIPv4Adress());
    addr.sin_port = htons(m_localAddress.port());

    if(::bind(m_socket, reinterpret_cast<const sockaddr *>(&addr), sizeof(sockaddr_in)) < 0) {
        close();
        return false;
    }

    // set non-blocking io
//#ifdef PLATFORM_MAC || PLATFORM_LINUX
//    int nonBlocking = 1;
//    if(fcntl(m_socket, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
//        close();
//        return false;
//    }
//#elif PLATFORM_WINDOWS
//    DWORD nonBlocking = 1;
//    if(ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0) {
//        close();
//        return false;
//    }
//#endif

    return true;
}

void Socket::close() {
    if(m_socket != 0) {
#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
        ::close(m_socket);
#else
        ::closesocket(m_socket);
#endif
        m_socket = 0;
    }
}

bool Socket::isValid() const {
    return m_socket != 0;
}

bool Socket::isDataAvailable() const {
    u_long flag = 0;
#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    ::ioctl(m_socket, FIONREAD, &flag);
#else
    ::ioctlsocket(m_socket, FIONREAD, &flag);
#endif

    return flag > 0;
}
