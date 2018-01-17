#ifndef AURI_H
#define AURI_H


#include <string>

#include "acommon.h"

using namespace std;

class AUriPrivate;

class NEXT_LIBRARY_EXPORT AUri {
public:
    AUri                        (const string &uri);

    string                      scheme                      () const;
    string                      host                        () const;
    string                      path                        () const;
    string                      query                       () const;
    string                      fragment                    () const;
    string                      name                        () const;

private:
    friend class AUriPrivate;

    AUriPrivate                *p_ptr;
};

#endif // AURI_H
