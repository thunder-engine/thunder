#include "networkaddress.h"

#include "socket.h"

#include <cstring>

/*!
    \class NetworkAddress
    \brief The NetworkAddress class is designed to encapsulate an IPv4 address and a port number.
    \inmodule Network

    The NetworkAddress class provides a way to represent an IP address and its associated port.
    It supports initialization via raw values (IP address in integer format or string format), and provides methods for comparison, conversion, and retrieval of IP address and port information.
*/
/*!
    Default constructor that initializes the address to 0 and port to 0.
*/
NetworkAddress::NetworkAddress() :
        m_address(0),
        m_port(0) {

}
/*!
    Constructor that initializes the NetworkAddress with the provided IP \a address and \a port.
*/
NetworkAddress::NetworkAddress(uint32_t address, uint16_t port) :
        m_address(htonl(address)),
        m_port(port) {

}
/*!
    Constructor that initializes the NetworkAddress using a string representation of the IP \a address (either hostname or numeric IP address) and the specified \a port.
*/
NetworkAddress::NetworkAddress(const String &address, uint16_t port) :
        m_address(0),
        m_port(port) {

    struct hostent *he = ::gethostbyname(address.data());
    if(he) {
        memcpy(&m_address, he->h_addr, he->h_length);
    } else {
        m_address = ::inet_addr(address.data());
    }
}
/*!
    Returns the IP address stored in the NetworkAddress object as a uint32_t value.
*/
uint32_t NetworkAddress::toIPv4Adress() const {
    return m_address;
}
/*!
    Returns the port number stored in the NetworkAddress object.
*/
uint16_t NetworkAddress::port() const {
    return m_port;
}
/*!
    Compares a this NetworkAddress with NetworkAddress \a right object.
    Returns true if addresses are equal; otherwise returns false.
*/
bool NetworkAddress::operator==(const NetworkAddress &right) const {
    return m_address == right.m_address && m_port == right.m_port;
}
/*!
    Compares a this NetworkAddress with NetworkAddress \a right object.
    Returns false if addresses are equal; otherwise returns true.
*/
bool NetworkAddress::operator!=(const NetworkAddress &right) const {
    return !(*this == right);
}
/*!
    Compares this NetworkAddress with \a right object to determine if the current object is "less than" the provided object based on the IP address alone.
*/
bool NetworkAddress::operator<(const NetworkAddress &right) const {
    return (m_address < right.m_address);
}
