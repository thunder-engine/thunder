cmake_minimum_required(VERSION 3.10)

project(resources)

if(desktop)
    # Install Engine and Editor Resources
    install(DIRECTORY ${RESOURCE_ROOT}
        DESTINATION "${SDK_PATH}/resources"
        FILES_MATCHING PATTERN "*"
    )

    # Install Qt plugins
    install(DIRECTORY ${QT_PLUGINS_DIR}/imageformats
        DESTINATION "${PLATFORM_PATH}/plugins"
    )

    install(DIRECTORY ${QT_PLUGINS_DIR}/platforms
        DESTINATION "${PLATFORM_PATH}/plugins"
    )

    install(DIRECTORY ${QT_PLUGINS_DIR}/mediaservice
        DESTINATION "${PLATFORM_PATH}/mediaservice"
    )

    if(LINUX)
        install(DIRECTORY ${QT_PLUGINS_DIR}/xcbglintegrations
            DESTINATION "${PLATFORM_PATH}/xcbglintegrations"
        )

        install(DIRECTORY ${LINUX_LIBS}
            DESTINATION "${PLATFORM_PATH}/xcbglintegrations"
        )
    endif()

    set(libPrefix "${QT_BIN_DIR}/")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(libSuffix "d")
    endif()

    if(WIN32)
        set(libSuffix "${libSuffix}.dll")
    endif()

    set(QT_LIBS
        "Qt5Core"
        "Qt5Gui"
        "Qt5Network"
        "Qt5Multimedia"
        "Qt5Svg"
        "Qt5Widgets"
    )

    if(LINUX)
        set(QT_LIBS
           ${QT_LIBS}
           "DBus"
           "XcbQpa"
        )
    endif()

    list(TRANSFORM QT_LIBS PREPEND ${libPrefix})
    if(WIN32)
        list(TRANSFORM QT_LIBS APPEND ${libSuffix})
    endif()

    if(LINUX)
        set(QT_LIBS
            ${QT_LIBS}
            "libicudata.so.56"
            "libicudata.so.56.1"
            "libicui18n.so.56"
            "libicui18n.so.56.1"
            "libicuuc.so.56"
            "libicuuc.so.56.1"
        )
    endif()

    # Install Qt libraries
    install(FILES ${QT_LIBS}
        DESTINATION "${BIN_PATH}"
    )

    # Install Qt config
    install(FILES "${CMAKE_SOURCE_DIR}/build/${OS_NAME}/qt.conf"
        DESTINATION "${BIN_PATH}"
    )

endif()
