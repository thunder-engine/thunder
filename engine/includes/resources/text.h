#ifndef TEXT_H
#define TEXT_H

#include "resource.h"

class ENGINE_EXPORT Text : public Resource {
    A_OBJECT(Text, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, size, Text::size, Text::setSize)
    )

    A_METHODS(
        A_METHOD(string, Text::text)
    )

public:
    Text();
    ~Text();

    uint32_t size() const;
    void setSize(uint32_t);

    std::string text();

    uint8_t *data();

protected:
    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

private:
    ByteArray m_data;

};

#endif // TEXT_H
