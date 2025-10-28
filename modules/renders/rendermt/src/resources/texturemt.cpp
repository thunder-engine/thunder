#include "resources/texturemt.h"

#include <cstring>

#include "commandbuffermt.h"

TextureMt::TextureMt() :
        m_native(nullptr),
        m_sampler(nullptr),
        m_buffer(nullptr) {

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
        MTL::CommandBuffer *cmd = WrapperMt::queue()->commandBuffer();
        MTL::BlitCommandEncoder *encoder = cmd->blitCommandEncoder();

        MTL::Origin readOrigin(x, y, 0);
        MTL::Size readSize(width, height, 1);

        int textSize = sizeRGB(m_width, m_height, 1);
        int rowSize = textSize / m_height;
        encoder->copyFromTexture(m_native, 0, 0, readOrigin, readSize, m_buffer, 0, rowSize, rowSize * m_height);

        encoder->endEncoding();
        cmd->commit();
        cmd->waitUntilCompleted();

        Surface &dst = surface(0);
        memcpy(dst[0].data(), m_buffer->contents(), rowSize * m_height);
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
        m_native->setLabel(NS::String::string(name().data(), NS::UTF8StringEncoding));

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

    if(isFeedback()) {
        if(m_buffer) {
            m_buffer->release();
        }

        m_buffer = WrapperMt::device()->newBuffer(sizeRGB(m_width, m_height, 1), MTL::ResourceStorageModeShared);
    }
}

void TextureMt::uploadTexture(uint32_t slice) {
    const Surface &image = surface(slice);

    bool cube = isCubemap();

    for(uint32_t i = 0; i < image.size(); i++) {
        uint32_t w = (m_width >> i);
        uint32_t h = (m_height >> i);
        uint32_t d = cube ? (m_depth >> i) : 1;

        int rowSize = w * components();
        switch(m_compress) {
            case Texture::BC1: rowSize = ((w + 3) / 4) * 8; break;
            case Texture::BC3:
            case Texture::BC7: rowSize = ((w + 3) / 4) * 16; break;
            case Texture::ASTC: rowSize = ((w + 3) / 4) * 16; break;
            case Texture::ETC1: rowSize = ((w + 3) / 4) * 8; break;
            case Texture::ETC2: rowSize = ((w + 3) / 4) * 16; break;
            case Texture::PVRTC: rowSize = ((std::max(w, 8U) + 3) / 4) * 8; break;
            default: break;
        }

        m_native->replaceRegion(MTL::Region(0, 0, 0, w, h, d), i, slice, image[i].data(), rowSize, image[i].size());
    }
}

MTL::PixelFormat TextureMt::pixelFormat() {
    if(m_compress) {
        switch(m_compress) {
            case Texture::BC1: return MTL::PixelFormatBC1_RGBA;
            case Texture::BC3: return MTL::PixelFormatBC3_RGBA;
            case Texture::BC7: return MTL::PixelFormatBC7_RGBAUnorm;
            case Texture::ASTC: return MTL::PixelFormatASTC_4x4_LDR;
            case Texture::ETC1: return MTL::PixelFormatETC2_RGB8;
            case Texture::ETC2: return MTL::PixelFormatETC2_RGB8A1;
            case Texture::PVRTC: return MTL::PixelFormatPVRTC_RGBA_4BPP;
            default: break;
        }
    } else {
        switch(m_format) {
            case R8: return MTL::PixelFormatR8Unorm;
            case RGB10A2: return MTL::PixelFormatRGB10A2Unorm;
            case R11G11B10Float: return MTL::PixelFormatRG11B10Float;
            case RGBA32Float: return MTL::PixelFormatRGBA32Float;
            case RGBA16Float: return MTL::PixelFormatRGBA16Float;
            case Depth: return MTL::PixelFormatDepth16Unorm;
            default: break;
        }
    }

    return MTL::PixelFormatRGBA8Unorm;
}
