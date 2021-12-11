import qbs

Project {
    id: modules

    references: [
        "media/media.qbs",
        "physics/bullet/bullet.qbs",
        "renders/renders.qbs",
        "vms/angel/angel.qbs",
        "gui/gui.qbs",
    ]
}
