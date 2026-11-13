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

#include "socket.h"

#include "networkaddress.h"

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
    unsigned long flag = 0;
#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    ::ioctl(m_socket, FIONREAD, &flag);
#else
    ::ioctlsocket(m_socket, FIONREAD, &flag);
#endif

    return flag > 0;
}
