#ifndef TEXT_H
#define TEXT_H

#include "engine.h"

class NEXT_LIBRARY_EXPORT Text : public AObject {
    A_REGISTER(Text, AObject, Resources)

public:
    Text                        ();

    virtual ~Text               ();

    const int8_t               *data                        () const;
    uint32_t                    size                        () const;

    string                      text                        () const;

protected:
    void                        loadUserData                (const AVariantMap &data);

protected:
    AByteArray                  m_Data;

};

#endif // TEXT_H
