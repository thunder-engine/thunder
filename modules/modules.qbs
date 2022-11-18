import qbs

Project {
    id: modules

    references: [
        "editor/editor.qbs",
        "media/media.qbs",
        "physics/bullet/bullet.qbs",
        "renders/renders.qbs",
        "vms/angel/angel.qbs"
    ]
}
