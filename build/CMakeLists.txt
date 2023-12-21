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

	# Install QML plugins
	install(DIRECTORY ${QT_QML_DIR}/QtGraphicalEffects
		DESTINATION "${PLATFORM_PATH}/qml"
	)

	install(DIRECTORY ${QT_QML_DIR}/QtQuick
		DESTINATION "${PLATFORM_PATH}/qml"
	)

	install(DIRECTORY ${QT_QML_DIR}/QtQuick.2
		DESTINATION "${PLATFORM_PATH}/qml"
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
		"Qt5QuickWidgets"
		"Qt5Quick"
		"Qt5QuickTemplates2"
		"Qt5QuickShapes"
		"Qt5QuickControls2"
		"Qt5Qml"
		"Qt5Svg"
		"Qt5Widgets"
		"Qt5Gamepad"
		"Qt5Script"
	)

	if (Qt5Core_VERSION VERSION_GREATER_EQUAL 5.14.0)
		list(APPEND QT_LIBS "Qt5QmlModels")
		list(APPEND QT_LIBS "Qt5QmlWorkerScript")
    endif()

	list(TRANSFORM QT_LIBS PREPEND ${libPrefix})
	list(TRANSFORM QT_LIBS APPEND ${libSuffix})

	# Install Qt libraries
	install(FILES ${QT_LIBS}
		DESTINATION "${BIN_PATH}"
	)

	# Install Qt config
	install(FILES "${CMAKE_SOURCE_DIR}/build/${OS_NAME}/qt.conf"
		DESTINATION "${BIN_PATH}"
	)

endif()