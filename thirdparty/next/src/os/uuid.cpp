#include "uuid.h"

#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <astring.h>

/*!
    \class Uuid
    \brief Universally unique identifier helper class.
    \since Next 1.0
    \inmodule OS

    `Uuid` represents a 128-bit UUID (RFC 4122 compatible). It provides
    utilities to create random (version 4) UUIDs, convert to/from string
    and byte array representations, and basic comparison operators.
*/

/*!
    The internal 16-byte value is zero-initialized.
*/
Uuid::Uuid() :
    data{} {

}

/*!
    Construct a \a uuid from its textual representation.

    Accepts common UUID formats with or without braces and dashes
    (for example: `{550e8400-e29b-41d4-a716-446655440000}` or `550e8400e29b41d4a716446655440000`).
    Non-hex characters except for separators are ignored.
    If the input contains fewer than 16 bytes, the remaining bytes stay zero.
*/
Uuid::Uuid(const TString &uuid) {
    size_t pos = 0;
    for(size_t i = 0; i < uuid.length(); ++i) {
        if(uuid.at(i) == '-' || uuid.at(i) == '{') continue;

        if(i + 1 >= uuid.length() || pos == 16) {
            break;
        }

        TString byteStr = uuid.mid(i, 2);
        data[pos++] = static_cast<uint8_t>(std::stoi(byteStr.toStdString(), nullptr, 16));
        i++;
    }
}

/*!
    Generate a random (version 4) UUID.

    Uses a C++ random device and Mersenne Twister generator to produce
    16 random bytes. Sets version and variant bits according to RFC 4122.
*/
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

/*!
    Check whether the UUID is the null UUID (all bytes zero).
*/
bool Uuid::isNull() const {
    return std::all_of(data.begin(), data.end(), [](uint8_t byte) { return byte == 0; });
}

/*!
    Return the canonical string representation of the UUID.

    The returned string is formatted with braces and hyphens, e.g. `{550e8400-e29b-41d4-a716-446655440000}`.
*/
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

/*!
    Convert the UUID to a 16-byte ByteArray.
*/
ByteArray Uuid::toByteArray() const {
    return ByteArray(data.begin(), data.end());
}

/*!
    Load UUID bytes from a 16-byte ByteArray.

    The provided \a array is expected to contain at least 16 bytes.
*/
void Uuid::fromByteArray(const ByteArray &array) {
    for(uint8_t i = 0; i < 16; i++) {
        data[i] = array[i];
    }
}

/*!
    Equality comparison of UUIDs.
*/
bool Uuid::operator== (const Uuid &other) const {
    return data == other.data;
}

/*!
    Inequality comparison of UUIDs.
*/
bool Uuid::operator!= (const Uuid &other) const {
    return !(*this == other);
}

/*!
    Strict weak ordering for UUIDs (lexicographical by bytes).
*/
bool Uuid::operator< (const Uuid &other) const {
    return data < other.data;
}
