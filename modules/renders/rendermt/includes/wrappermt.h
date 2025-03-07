#ifndef WRAPPERMT_H
#define WRAPPERMT_H

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class WrapperMt {
public:
    static MTL::Device *device();

protected:
    static MTL::Device *s_device;

};

#endif // WRAPPERMT_H
