import qbs

Project {
    id: thirdparty

    references: [
        "next/next.qbs",
        "zlib/zlib.qbs",
        "physfs/physfs.qbs",
        "glfw/glfw.qbs",
        "quazip/quazip.qbs"
    ]


}
