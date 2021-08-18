#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "postprocessor.h"

class Engine;

class AntiAliasing : public PostProcessor {
public:
    AntiAliasing();

    uint32_t layer() const override;
};

#endif // ANTIALIASING_H
