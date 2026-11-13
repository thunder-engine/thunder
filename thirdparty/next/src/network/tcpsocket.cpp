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

#include "tcpsocket.h"

#include "networkaddress.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

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

#include <log.h>

TcpSocket::TcpSocket(bool ssl) :
        Socket(),
        m_ssl(nullptr) {

    m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(ssl) {
        static SSL_CTX *ctx = nullptr;
        if(ctx == nullptr) {
            SSL_library_init();
            SSLeay_add_ssl_algorithms();
            SSL_load_error_strings();

            const SSL_METHOD *method = TLS_client_method();
            ctx = SSL_CTX_new(method);
        }

        m_ssl = SSL_new(ctx);
        if(m_ssl) {
            SSL_set_fd(m_ssl, m_socket);
        }
    }
}

bool TcpSocket::connect(const NetworkAddress &address) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = address.toIPv4Adress();
    addr.sin_port = htons(address.port());

    bool result = ::connect(m_socket, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == 0;
    if(result && m_ssl) {
        int err = SSL_connect(m_ssl);
        if(err <= 0) {
            aDebug() << "Error creating SSL connection:" << ERR_error_string(err, 0);
        }
    }

    return result;
}

void TcpSocket::disconnect() {
    close();
}

uint64_t TcpSocket::read(ByteArray &data) {
    if(m_ssl) {
        return SSL_read(m_ssl, reinterpret_cast<char *>(data.data()), data.size());
    }
    return ::recv(m_socket, reinterpret_cast<char *>(data.data()), data.size(), 0);
}

uint64_t TcpSocket::write(const ByteArray &data) {
    if(m_ssl) {
        return SSL_write(m_ssl, reinterpret_cast<const char *>(data.data()), data.size());
    }
    return ::send(m_socket, reinterpret_cast<const char *>(data.data()), data.size(), 0);
}
