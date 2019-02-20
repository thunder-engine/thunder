set(SRC_FILES
    src/glad.c
)

add_library(glad STATIC ${SRC_FILES})
target_include_directories(glad PUBLIC include)
