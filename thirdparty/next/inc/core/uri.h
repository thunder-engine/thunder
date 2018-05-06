#ifndef URI_H
#define URI_H

#include <string>

#include "common.h"

using namespace std;

class UriPrivate;

class NEXT_LIBRARY_EXPORT Uri {
public:
    Uri                         (const string &uri);

    ~Uri                        ();

    string                      scheme                      () const;
    string                      host                        () const;
    string                      path                        () const;
    string                      query                       () const;
    string                      fragment                    () const;
    string                      dir                         () const;
    string                      name                        () const;
    string                      baseName                    () const;
    string                      suffix                      () const;

private:
    friend class UriPrivate;

    UriPrivate                 *p_ptr;
};

#endif // URI_H
