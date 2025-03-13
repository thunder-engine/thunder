#ifndef WRAPPERMT_H
#define WRAPPERMT_H

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class WrapperMt {
public:
    static MTL::Device *device();

    static MTL::CommandQueue *queue();

protected:
    static MTL::Device *s_device;

    static MTL::CommandQueue *s_queue;

};

#endif // WRAPPERMT_H
