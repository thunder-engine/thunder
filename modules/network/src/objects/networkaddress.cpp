#include "networkaddress.h"

#include "socket.h"

#include <cstring>

NetworkAddress::NetworkAddress() :
        m_address(0),
        m_port(0) {

}

NetworkAddress::NetworkAddress(uint32_t address, uint16_t port) :
        m_address(htonl(address)),
        m_port(port) {

}

NetworkAddress::NetworkAddress(const std::string &address, uint16_t port) :
        m_address(0),
        m_port(port) {

    struct hostent *he = ::gethostbyname(address.c_str());
    if(he) {
        memcpy(&m_address, he->h_addr, he->h_length);
    } else {
        m_address = ::inet_addr(address.c_str());
    }
}

uint32_t NetworkAddress::toIPv4Adress() const {
    return m_address;
}

uint16_t NetworkAddress::port() const {
    return m_port;
}

bool NetworkAddress::operator==(const NetworkAddress &right) const {
    return m_address == right.m_address && m_port == right.m_port;
}

bool NetworkAddress::operator!=(const NetworkAddress &right) const {
    return !(*this == right);
}

bool NetworkAddress::operator<(const NetworkAddress &right) const {
    return (m_address < right.m_address);
}
