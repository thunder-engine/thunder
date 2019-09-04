import qbs

Project {
    id: thirdparty

    references: [
		"angelscript/angelscript.qbs",
        "bullet/bullet3.qbs",
        "next/next.qbs",
        "zlib/zlib.qbs",
        "physfs/physfs.qbs",
        "glfw/glfw.qbs",
        "glfm/glfm.qbs",
		"glsl/glsl.qbs",
        "spirvcross/spirvcross.qbs",
        "libogg/ogg.qbs",
        "libvorbis/vorbis.qbs",
        "quazip/quazip.qbs",
        "glad/glad.qbs",
        "freetype/freetype.qbs",
        "syntaxhighlighting/syntaxhighlighting.qbs"
    ]
}
