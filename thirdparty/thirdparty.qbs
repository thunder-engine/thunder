import qbs

Project {
    id: thirdparty

    references: [
		"basisu/basisu.qbs",
        "assimp/assimp.qbs",
        "next/next.qbs",
        "zlib/zlib.qbs",
        "poly2tri/poly2tri.qbs",
		"pugixml/pugixml.qbs",
        "glfw/glfw.qbs",
        "glfm/glfm.qbs",
        "glsl/glsl.qbs",
		"gtest/gtest.qbs",
        "spirvcross/spirvcross.qbs",
        "minizip/minizip.qbs",
        "syntaxhighlighting/syntaxhighlighting.qbs"
    ]
}
