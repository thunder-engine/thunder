# import qbs
# 
# Project {
#     id: thirdparty
# 
#     references: [
#         "angelscript/angelscript.qbs"
add_subdirectory(angelscript)
#         "assimp/assimp.qbs",
add_subdirectory(assimp)
#         "bullet/bullet3.qbs",
add_subdirectory(bullet)
#         "next/next.qbs",
add_subdirectory(next)
#         "zlib/zlib.qbs",
add_subdirectory(zlib)
#         "physfs/physfs.qbs",
add_subdirectory(physfs)
#         "poly2tri/poly2tri.qbs",
add_subdirectory(poly2tri)
#         "glfw/glfw.qbs",
add_subdirectory(glfw)
#         "glfm/glfm.qbs",
add_subdirectory(glfm)
#         "glsl/glsl.qbs",
add_subdirectory(glsl)
#         "irrXML/irrXML.qbs",
add_subdirectory(irrXML)
#         "spirvcross/spirvcross.qbs",
add_subdirectory(spirvcross)
#         "libogg/ogg.qbs",
add_subdirectory(libogg)
#         "libvorbis/vorbis.qbs",
add_subdirectory(libvorbis)
#         "quazip/quazip.qbs",
add_subdirectory(quazip)
#         "glad/glad.qbs",
add_subdirectory(glad)
#         "freetype/freetype.qbs",
add_subdirectory(freetype)
#         "syntaxhighlighting/syntaxhighlighting.qbs"
add_subdirectory(syntaxhighlighting)
#     ]
# }
add_subdirectory(utf8cpp)
