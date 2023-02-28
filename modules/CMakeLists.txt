# import qbs
# 
# Project {
#     id: modules
# 
#     references: [
#         "editor/editor.qbs",
#         "media/media.qbs",
#         "physics/bullet/bullet.qbs",
#         "renders/renders.qbs",
#         "vms/angel/angel.qbs"
#     ]
# }

add_subdirectory(editor)
add_subdirectory(media)
add_subdirectory(physics/bullet)
add_subdirectory(renders)
add_subdirectory(vms/angel)
