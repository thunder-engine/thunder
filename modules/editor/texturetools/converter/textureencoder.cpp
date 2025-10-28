#include <encoder/basisu_comp.h>

#include <resources/texture.h>

#include "textureconverter.h"

uint32_t estimateTranscodedSize(uint32_t width, uint32_t height, basist::transcoder_texture_format format) {
    switch(format) {
    case basist::transcoder_texture_format::cTFBC1_RGB:
        return ((width + 3) / 4) * ((height + 3) / 4) * 8;
    case basist::transcoder_texture_format::cTFBC3_RGBA:
    case basist::transcoder_texture_format::cTFBC7_RGBA:
        return ((width + 3) / 4) * ((height + 3) / 4) * 16;
    case basist::transcoder_texture_format::cTFASTC_4x4_RGBA:
        return ((width + 3) / 4) * ((height + 3) / 4) * 16;
    case basist::transcoder_texture_format::cTFETC1_RGB:
    case basist::transcoder_texture_format::cTFETC2_RGBA:
        return ((width + 3) / 4) * ((height + 3) / 4) * 8;
    case basist::transcoder_texture_format::cTFPVRTC1_4_RGB:
    case basist::transcoder_texture_format::cTFPVRTC1_4_RGBA:
        return std::max(width * height * 2 / 8, 32U);
    default: break;
    }

    return width * height * 4;
}

bool TextureConverter::compress(Texture *texture) {
    bool result = basisu::basisu_encoder_init();

    if(result) {
        basisu::job_pool jpool(std::thread::hardware_concurrency());

        basisu::basis_compressor_params params;
        params.m_multithreading = true;
        params.m_pJob_pool = &jpool;

        params.m_uastc = true;
        params.m_pack_uastc_ldr_4x4_flags = basisu::cPackUASTCLevelDefault;
        params.m_rdo_uastc_ldr_4x4_quality_scalar = 1.0f;

        if(texture->isCubemap()) {
            params.m_tex_type = basist::cBASISTexTypeCubemapArray;
        }

        if(texture->depth() > 1) {
            params.m_tex_type = basist::cBASISTexTypeVolume;
        }

        uint32_t w = texture->width();
        uint32_t h = texture->height();

        uint32_t channels = 4;
        if(texture->format() == Texture::RGB8) {
            channels = 3;
        }

        basist::basisu_transcoder transcoder;

        result = false;
        for(uint32_t side = 0; side < texture->sides(); side++) {
            Texture::Surface &surface = texture->surface(side);

            for(uint32_t lod = 0; lod < surface.size(); lod++) {
                params.m_source_images.push_back(basisu::image(surface[lod].data(), (w >> lod), (h >> lod), channels));

                basisu::basis_compressor comp;
                if(comp.init(params)) {
                    if(comp.process() == basisu::basis_compressor::cECSuccess) {
                        const basisu::uint8_vec &data = comp.get_output_basis_file();

                        result = transcoder.start_transcoding(data.data(), data.size());

                        basist::basisu_file_info fileInfo;
                        if(!transcoder.get_file_info(data.data(), data.size(), fileInfo)) {
                            return false;
                        }

                        basist::basisu_image_info levelInfo;
                        if(!transcoder.get_image_info(data.data(), data.size(), levelInfo, lod)) {
                            continue;
                        }

                        basist::transcoder_texture_format format = basist::transcoder_texture_format::cTFRGBA32;
                        switch(texture->compress()) {
                            case Texture::BC1: format = basist::transcoder_texture_format::cTFBC1_RGB; break;
                            case Texture::BC3: format = basist::transcoder_texture_format::cTFBC3_RGBA; break;
                            case Texture::BC7: format = basist::transcoder_texture_format::cTFBC7_RGBA; break;
                            case Texture::ASTC: format = basist::transcoder_texture_format::cTFASTC_4x4_RGBA; break;
                            case Texture::ETC1: format = basist::transcoder_texture_format::cTFETC1_RGB; break;
                            case Texture::ETC2: format = basist::transcoder_texture_format::cTFETC2_RGBA; break;
                            case Texture::PVRTC: format = basist::transcoder_texture_format::cTFPVRTC1_4_RGBA; break;
                            default: break;
                        }

                        surface[lod].resize(estimateTranscodedSize((w >> lod), (h >> lod), format));

                        result = transcoder.transcode_image_level(data.data(), data.size(), side, lod, surface[lod].data(), surface[lod].size(), format);

                        transcoder.stop_transcoding();
                    }
                }
            }
        }

        basisu::basisu_encoder_deinit();
    }

    return result;
}
