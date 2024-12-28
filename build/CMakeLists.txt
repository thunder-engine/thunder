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

	set(libPrefix "${QT_BIN_DIR}/")

	if(WIN32)
		set(libSuffix "$<$<CONFIG:Debug>:d>.dll")
	endif()

	set(QT_LIBS
		"Qt5Core"
		"Qt5Gui"
		"Qt5Xml"
		"Qt5XmlPatterns"
		"Qt5Network"
		"Qt5Multimedia"
		"Qt5Svg"
		"Qt5Widgets"
		"Qt5Script"
	)

	list(TRANSFORM QT_LIBS PREPEND ${libPrefix})
	if(WIN32)
	    list(TRANSFORM QT_LIBS APPEND ${libSuffix})
    endif()

    list(TRANSFORM QT_LIBS PREPEND ${libPrefix})
    if(WIN32)
        list(TRANSFORM QT_LIBS APPEND ${libSuffix})
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
