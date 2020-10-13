#ifndef TEXT_H
#define TEXT_H

#include "resource.h"

class TextPrivate;

class NEXT_LIBRARY_EXPORT Text : public Resource {
    A_REGISTER(Text, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, size,  Text::size, Text::setSize)
    )

    A_METHODS(
        A_METHOD(string, Text::text)
    )

public:
    Text ();
    ~Text ();

    uint32_t size () const;
    void setSize (uint32_t);

    string text ();

protected:
    void loadUserData (const VariantMap &data) override;

    char *data() const;

private:
    TextPrivate *p_ptr;

};

#endif // TEXT_H
