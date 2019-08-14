#ifndef TEXT_H
#define TEXT_H

#include "resource.h"

class TextPrivate;

class NEXT_LIBRARY_EXPORT Text : public Resource {
    A_REGISTER(Text, Resource, Resources)

public:
    Text ();
    ~Text ();

    char *data() const;

    uint32_t size () const;
    void setSize (uint32_t);

    string text ();

private:
    void loadUserData (const VariantMap &data) override;

private:
    TextPrivate *p_ptr;

};

#endif // TEXT_H
