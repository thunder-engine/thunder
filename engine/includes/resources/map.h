#ifndef MAP_H
#define MAP_H

#include "prefab.h"

class NEXT_LIBRARY_EXPORT Map : public Prefab {
    A_REGISTER(Map, Prefab, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
};

#endif // MAP_H
