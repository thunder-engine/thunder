import qbs
import qbs.FileInfo

Project {
    id: glsl
    property stringList srcFiles: [
        "glslang/GenericCodeGen/**/*.cpp",
        "glslang/MachineIndependent/**/*.cpp",
        "glslang/Public/**/*.cpp",
        "SPIRV/**/*.cpp",
        "glslang/GenericCodeGen/**/*.h",
        "glslang/MachineIndependent/**/*.h",
        "glslang/Public/**/*.h",
        "glslang/Include/**/*.h",
        "glslang/OSDependent/**/*.h",
        "SPIRV/**/*.h",
        "OGLCompilersDLL/*.cpp"
    ]

    property stringList incPaths: [
        "./",
        "OGLCompilersDLL"
    ]

    StaticLibrary {
        name: "glsl"
        condition: glsl.desktop
        files: glsl.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: glsl.incPaths
        cpp.libraryPaths: [ ]
        cpp.cxxLanguageVersion: glsl.languageVersion
        cpp.cxxStandardLibrary: glsl.standardLibrary
        cpp.minimumMacosVersion: glsl.osxVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
            files: outer.concat(["glslang/OSDependent/Windows/ossource.cpp"])
        }
        
        Properties {
            condition: qbs.targetOS.contains("unix")
            files: outer.concat(["glslang/OSDependent/Unix/ossource.cpp"])
        }
    }
}
