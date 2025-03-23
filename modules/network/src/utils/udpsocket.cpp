#include "udpsocket.h"

#include "networkaddress.h"

bool UdpSocket::bind(const NetworkAddress &address) {
    m_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return Socket::bind(address);
}

uint64_t UdpSocket::read(ByteArray &data, NetworkAddress *address) {
    if(m_socket == 0) {
        return 0;
    }

    uint64_t receivedBytes = 0;

    if(address) {
#ifdef PLATFORM_WINDOWS
        typedef int socklen_t;
#endif
        sockaddr_in from;
        socklen_t length = sizeof(sockaddr_in);

        receivedBytes = ::recvfrom(m_socket, reinterpret_cast<char *>(data.data()), data.size(), 0, reinterpret_cast<sockaddr *>(&from), &length);

        *address = NetworkAddress(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));
    } else {
        receivedBytes = ::recvfrom(m_socket, reinterpret_cast<char *>(data.data()), data.size(), 0, nullptr, nullptr);
    }

    return MAX(receivedBytes, 0);
}

uint64_t UdpSocket::write(const ByteArray &data, const NetworkAddress &address) {
    if(m_socket) {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(address.toIPv4Adress());
        addr.sin_port = htons(address.port());

        return ::sendto(m_socket, reinterpret_cast<const char *>(data.data()), data.size(), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr_in));
    }

    return 0;
}
