#include "uuid.h"

#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <astring.h>

Uuid::Uuid() :
    data{} {

}

Uuid::Uuid(const TString &uuid) {
    size_t pos = 0;
    for(size_t i = 0; i < uuid.length(); ++i) {
        if(uuid.at(i) == '-' || uuid.at(i) == '{') continue;

        if(i + 1 >= uuid.length() || pos == 15) {
            break;
        }

        TString byteStr = uuid.mid(i, 2);
        data[pos++] = static_cast<uint8_t>(std::stoi(byteStr.toStdString(), nullptr, 16));
        i++;
    }
}

Uuid Uuid::createUuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 255);

    Uuid result;
    for(auto &byte : result.data) {
        byte = dis(gen);
    }

    // Set version (4) and variant (10)
    result.data[6] = (result.data[6] & 0x0F) | 0x40; // version 4
    result.data[8] = (result.data[8] & 0x3F) | 0x80; // variant RFC 4122

    return result;
}

bool Uuid::isNull() const {
    return std::all_of(data.begin(), data.end(), [](uint8_t byte) { return byte == 0; });
}

TString Uuid::toString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    oss << '{';
    for(uint8_t i = 0; i < 4; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    oss << '-';

    for(uint8_t i = 4; i < 6; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    oss << '-';

    // time_hi_and_version (2 bytes)
    for(uint8_t i = 6; i < 8; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    oss << '-';

    for(uint8_t i = 8; i < 10; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    oss << '-';

    for(uint8_t i = 10; i < 16; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    oss << '}';

    return oss.str();
}

ByteArray Uuid::toByteArray() const {
    return ByteArray(data.begin(), data.end());
}

bool Uuid::operator== (const Uuid &other) const {
    return data == other.data;
}

bool Uuid::operator!= (const Uuid &other) const {
    return !(*this == other);
}

bool Uuid::operator< (const Uuid &other) const {
    return data < other.data;
}
