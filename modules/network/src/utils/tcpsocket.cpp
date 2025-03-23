#include "tcpsocket.h"

#include "networkaddress.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

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

    bool result = ::connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) == 0;
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
