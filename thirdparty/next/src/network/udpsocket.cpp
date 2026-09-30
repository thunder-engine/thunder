/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "udpsocket.h"

#include "networkaddress.h"
#include "amath.h"

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
