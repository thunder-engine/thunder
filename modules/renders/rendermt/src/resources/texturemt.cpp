#include "resources/texturemt.h"

#include <cstring>

#include "commandbuffermt.h"

TextureMt::TextureMt() :
        m_native(nullptr),
        m_sampler(nullptr) {

}

MTL::Texture *TextureMt::nativeHandle() {
    switch(state()) {
        case Unloading: {
            if(m_native) {
                m_native->release();
                m_native = nullptr;
            }

            if(m_sampler) {
                m_sampler->release();
                m_sampler = nullptr;
            }

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            if(m_native) {
                m_native->release();
                m_native = nullptr;
            }

            updateTexture();

            switchState(Ready);
        } break;
        default: break;
    }

    return m_native;
}

MTL::SamplerState *TextureMt::sampler() {
    return m_sampler;
}

void TextureMt::readPixels(int x, int y, int width, int height) {
    if(sides() != 0) {
        bool depth = (TextureMt::format() == Depth);
    }
}

void TextureMt::updateTexture() {
    if(m_native == nullptr) {
        MTL::TextureDescriptor *textureDesc = MTL::TextureDescriptor::alloc()->init();
        textureDesc->setWidth(m_width);
        textureDesc->setHeight(m_height);
        textureDesc->setDepth(m_depth);
        textureDesc->setPixelFormat(pixelFormat());
        if(isCubemap()) {
            textureDesc->setTextureType(MTL::TextureTypeCube);
        } else {
            textureDesc->setTextureType(m_depth == 1 ? MTL::TextureType2D : MTL::TextureType3D);
        }
        textureDesc->setStorageMode(MTL::StorageModeManaged);
        textureDesc->setUsage(isRender() ?
                                  (MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead) :
                                  (MTL::ResourceUsageSample | MTL::ResourceUsageRead));
        textureDesc->setMipmapLevelCount(mipCount());

        m_native = WrapperMt::device()->newTexture(textureDesc);
        m_native->setLabel(NS::String::string(name().c_str(), NS::UTF8StringEncoding));

        textureDesc->release();
    }

    if(m_sampler == nullptr) {
        MTL::SamplerDescriptor *samplerDesc = MTL::SamplerDescriptor::alloc()->init();

        MTL::SamplerMinMagFilter min = MTL::SamplerMinMagFilterNearest;
        MTL::SamplerMinMagFilter mag = MTL::SamplerMinMagFilterNearest;

        switch(filtering()) {
            case Trilinear:
            case Bilinear:  mag = MTL::SamplerMinMagFilterLinear; min = MTL::SamplerMinMagFilterLinear; break;
            default: break;
        }
        samplerDesc->setMinFilter(min);
        samplerDesc->setMagFilter(mag);

        if(mipCount() > 1) {
            samplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
        }

        MTL::SamplerAddressMode wrap = MTL::SamplerAddressModeClampToEdge;

        if(isCubemap()) {
            wrap = MTL::SamplerAddressModeMirrorRepeat;
        } else {
            switch(TextureMt::wrap()) {
                case Repeat: wrap = MTL::SamplerAddressModeRepeat; break;
                case Mirrored: wrap = MTL::SamplerAddressModeMirrorRepeat; break;
                default: break;
            }
        }

        samplerDesc->setSAddressMode(wrap);
        samplerDesc->setTAddressMode(wrap);
        samplerDesc->setRAddressMode(wrap);

        m_sampler = WrapperMt::device()->newSamplerState(samplerDesc);

        samplerDesc->release();
    }

    if(!isRender()) {
        if(isCubemap()) {
            for(uint32_t n = 0; n < 6; n++) {
                uploadTexture(n);
            }
        } else {
            uploadTexture(0);
        }
    }
}

void TextureMt::uploadTexture(uint32_t slice) {
    const Surface &image = surface(slice);
    int bpp = 4; // bytes per pixel (probably issues with compressed formats)

    for(uint32_t i = 0; i < image.size(); i++) {
        int32_t w = (m_width >> i);
        int32_t h = (m_height >> i);
        int32_t d = (m_depth >> i);
        m_native->replaceRegion(MTL::Region(0, 0, 0, w, h, d), i, slice, image[i].data(), w * bpp, image[i].size());
    }
}

MTL::PixelFormat TextureMt::pixelFormat() {
    switch(m_format) {
        case R8: return MTL::PixelFormatR8Unorm;
        case RGB10A2: return MTL::PixelFormatRGB10A2Unorm;
        case R11G11B10Float: return MTL::PixelFormatRG11B10Float;
        case RGBA32Float: return MTL::PixelFormatRGBA32Float;
        case RGBA16Float: return MTL::PixelFormatRGBA16Float;
        case Depth: return MTL::PixelFormatDepth16Unorm;
        default: break;
    }

    return MTL::PixelFormatRGBA8Unorm;
}
