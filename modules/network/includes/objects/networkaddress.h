#ifndef NETWORKADDRESS_H
#define NETWORKADDRESS_H

#include <cstdint>

#include <network.h>

class NETWORK_EXPORT NetworkAddress {
public:
    NetworkAddress();

    NetworkAddress(uint32_t address, uint16_t port);

    NetworkAddress(const std::string &address, uint16_t port);

    uint32_t toIPv4Adress() const;

    uint16_t port() const;

    bool operator==(const NetworkAddress &right) const;

    bool operator!=(const NetworkAddress &right) const;

    bool operator<(const NetworkAddress &right) const;

private:
    uint32_t m_address;

    uint16_t m_port;

};

#endif // NETWORKADDRESS_H
