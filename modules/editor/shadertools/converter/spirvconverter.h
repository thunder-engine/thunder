#ifndef SPIRVCONVERTER_H
#define SPIRVCONVERTER_H

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <OGLCompilersDLL/InitializeDll.h>

#include <log.h>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>
#include <spirv_hlsl.hpp>

static spirv_cross::CompilerGLSL::Options options;

const TBuiltInResource defaultResource = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,
        /* .maxDualSourceDrawBuffersEXT = */ 1,

        /* .limits = */ {
            /* .nonInductiveForLoops = */ 1,
            /* .whileLoops = */ 1,
            /* .doWhileLoops = */ 1,
            /* .generalUniformIndexing = */ 1,
            /* .generalAttributeMatrixVectorIndexing = */ 1,
            /* .generalVaryingIndexing = */ 1,
            /* .generalSamplerIndexing = */ 1,
            /* .generalVariableIndexing = */ 1,
            /* .generalConstantMatrixVectorIndexing = */ 1,
        }
    };

class SpirVConverter  {
public:
    enum InputType {
        Stage,
        Uniform,
        Image
    };

    struct Input {
        int32_t format;

        int32_t location;

        int32_t type;

        int32_t size;

        TString name;

    };
    typedef std::vector<Input> Inputs;

    static std::vector<uint32_t> glslToSpv(const TString &buff, Inputs &inputs, EShLanguage stage) {
        ShInitialize();

        glslang::TProgram program;

        EShMessages messages = EShMsgSpvRules;
        TBuiltInResource resources = defaultResource;

        glslang::TShader shader(stage);

        const char *str = buff.data();
        shader.setStrings(&str, 1);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2);

        const int defaultVersion = 450;
        if(shader.parse(&resources, defaultVersion, false, messages)) {
            program.addShader(&shader);

            if(program.link(messages)) {
                if(program.getIntermediate(stage)) {
                    glslang::SpvOptions spvOptions;
                    spvOptions.generateDebugInfo = false;
                    spvOptions.disableOptimizer = false;
                    spvOptions.optimizeSize = true;
                    spvOptions.disassemble = false;
                    spvOptions.validate = false;

                    std::vector<uint32_t> spirv;
                    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spvOptions);

                    spirv_cross::CompilerGLSL glsl(spirv);
                    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

                    if(stage == EShLangVertex) {
                        for(int32_t i = 0; i < resources.stage_inputs.size(); i++) {
                            Input input;

                            input.name = glsl.get_name(resources.stage_inputs[i].id);
                            input.location = glsl.get_decoration(resources.stage_inputs[i].id, spv::DecorationLocation);
                            input.type = Stage;

                            spirv_cross::SPIRType type = glsl.get_type(resources.stage_inputs[i].type_id);
                            if(type.basetype == spirv_cross::SPIRType::Float) {
                                switch(type.vecsize) {
                                    case 1: input.format = MetaType::FLOAT; break;
                                    case 2: input.format = MetaType::VECTOR2; break;
                                    case 3: input.format = MetaType::VECTOR3; break;
                                    case 4: input.format = MetaType::VECTOR4; break;
                                    default: break;
                                }
                            }

                            inputs.push_back(input);
                        }
                    }

                    for(int32_t i = 0; i < resources.uniform_buffers.size(); i++) {
                        Input input;

                        input.name = glsl.get_name(resources.uniform_buffers[i].id);
                        input.location = glsl.get_decoration(resources.uniform_buffers[i].id, spv::DecorationLocation);
                        input.type = Uniform;

                        inputs.push_back(input);
                    }

                    for(int32_t i = 0; i < resources.storage_buffers.size(); i++) {
                        Input input;

                        input.name = glsl.get_name(resources.storage_buffers[i].id);
                        input.location = glsl.get_decoration(resources.storage_buffers[i].id, spv::DecorationLocation);
                        input.type = Uniform;

                        inputs.push_back(input);
                    }

                    return spirv;
                }
            } else {
                aError() << "[Program]" << program.getInfoLog();
            }
        } else {
            aError() << "[Shader]" << shader.getInfoLog();
            aError() << buff;
        }
        return std::vector<uint32_t>();
    }

    static void setGlslVersion(uint32_t version, bool es) {
        options.version = version;
        options.es = es;
        options.separate_shader_objects = false;
    }

    static TString spvToGlsl(std::vector<uint32_t> spv) {
        spirv_cross::CompilerGLSL glsl(spv);
        glsl.set_common_options(options);

        return glsl.compile();
    }

    static TString spvToMetal(std::vector<uint32_t> spv, SpirVConverter::Inputs &inputs, EShLanguage stage) {
        spirv_cross::CompilerMSL msl(spv);

        spirv_cross::CompilerMSL::Options options;
        options.platform = spirv_cross::CompilerMSL::Options::macOS;
        msl.set_msl_options(options);

        TString result = msl.compile();

        spirv_cross::ShaderResources resources = msl.get_shader_resources();

        for(int32_t i = 0; i < resources.uniform_buffers.size(); i++) {
            int id = resources.uniform_buffers[i].id;
            int bind = msl.get_automatic_msl_resource_binding(id);
            auto type = msl.get_type(resources.uniform_buffers[i].type_id);
            TString name = msl.get_name(id);
            for(auto &it : inputs) {
                if(it.name == name && it.type == Uniform) {
                    it.location = bind;
                    it.size = msl.get_declared_struct_size(type);
                }
            }
        }

        for(int32_t i = 0; i < resources.storage_buffers.size(); i++) {
            int id = resources.storage_buffers[i].id;
            int bind = msl.get_automatic_msl_resource_binding(id);
            auto type = msl.get_type(resources.uniform_buffers[i].type_id);
            TString name = msl.get_name(id);
            for(auto &it : inputs) {
                if(it.name == name && it.type == Uniform) {
                    it.location = bind;
                    it.size = msl.get_declared_struct_size(type);
                }
            }
        }

        for(int32_t i = 0; i < resources.sampled_images.size(); i++) {
            int id = resources.sampled_images[i].id;

            Input image;
            image.type = Image;
            image.location = msl.get_automatic_msl_resource_binding(id);
            image.name = msl.get_name(id);

            inputs.push_back(image);
        }

        return result;
    }

    static TString spvToHlsl(std::vector<uint32_t> spv) {
        spirv_cross::CompilerHLSL hlsl(spv);

        spirv_cross::CompilerHLSL::Options options;
        hlsl.set_hlsl_options(options);

        return hlsl.compile();
    }
};

#endif // SPIRVCONVERTER_H
