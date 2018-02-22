import qbs

Project {
    id: thirdparty

    references: [
        "next/next.qbs",
        "zlib/zlib.qbs",
        "physfs/physfs.qbs",
        "glfw/glfw.qbs",
        "glfm/glfm.qbs",
        "quazip/quazip.qbs",
        "glad/glad.qbs"
    ]
}
