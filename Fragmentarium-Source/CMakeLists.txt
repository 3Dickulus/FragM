IF(UNIX)
cmake_minimum_required( VERSION 3.2 )
cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0071 NEW)
ELSE ()
cmake_minimum_required( VERSION 2.8 )
cmake_policy(SET CMP0048 NEW)
ENDIF ()

set(FRAGM_VERSION "2.5.0")

project(Fragmentarium-${FRAGM_VERSION} VERSION ${FRAGM_VERSION} LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

IF(UNIX AND NOT APPLE)
    # for find_package ( OpenEXR REQUIRED )
    find_package(ECM REQUIRED NO_MODULE)

    set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        )
ENDIF ()

set ( CMAKE_BUILD_TYPE Release )
set ( CMAKE_AUTOMOC ON )
set ( CMAKE_AUTOUIC ON)
set ( CMAKE_AUTORCC ON)
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

# find_package ( glm REQUIRED )
find_package ( OpenGL REQUIRED )
find_package ( Qt5LinguistTools REQUIRED )

find_package ( Qt5 COMPONENTS Core OpenGL Widgets Gui Xml Script ScriptTools REQUIRED )

# ON if you want opengl 4+ capabilities
OPTION ( USE_OPENGL_4 "Build with OpenGL 4.1" ON)
MARK_AS_ADVANCED (USE_OPENGL_4)
IF ( USE_OPENGL_4 )
    ADD_DEFINITIONS ( -DUSE_OPENGL_4=1 )
ENDIF ()

ADD_DEFINITIONS ( -DQT_DEVICE_PIXEL_RATIO=1 -Wno-deprecated )

# ON if you have OpenEXR libs and include files
OPTION ( USE_OPEN_EXR "Build with OpenEXR" ON)

OPTION ( BUILD_WIN_ZIP "Build Windows zip package" OFF)
OPTION ( BUILD_LINUX_DEB "Build Linux Debian package" OFF)
OPTION ( BUILD_LINUX_RPM "Build Linux RPM package" OFF)

# set to the mingw lib path
IF ( WIN32 )
  set ( MINGW_LIB_PATH "C:\\Qt/Tools/mingw492_32/i686-w64-mingw32/lib" CACHE PATH "set to the mingw lib path" )
ENDIF ()

IF(OPENGL_LIBRARIES_NOTFOUND)
    IF(APPLE)
        FIND_LIBRARY(OPENGL_LIBRARIES OpenGL )
    ELSEIF(UNIX)
        FIND_LIBRARY(OPENGL_LIBRARIES GL )
    ELSEIF(WIN32)
        set(OPENGL_LIBRARIES "-lopengl32" )
    ENDIF ()
ENDIF ()

MARK_AS_ADVANCED (OPENGL_LIBRARIES)

set (OpenEXR_LIBRARIES "")
set (OpenEXR_INCLUDE_DIRS "")
set (Ilmbase_INCLUDE_DIRS "")

    IF ( APPLE OR WIN32  )
        set ( GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../glm )
    ENDIF ()

MARK_AS_ADVANCED (OpenEXR_LIBRARIES)
MARK_AS_ADVANCED (OpenEXR_INCLUDE_DIRS)
MARK_AS_ADVANCED (Ilmbase_INCLUDE_DIRS)
MARK_AS_ADVANCED (GLM_INCLUDE_DIRS)

IF ( USE_OPEN_EXR )

    IF(UNIX AND NOT APPLE)
        find_package ( OpenEXR REQUIRED )
        set( OpenEXR_LIBS
            IlmImf
            Half
            Imath
            Iex
            IexMath
            IlmThread
        )

        set( OpenEXR_LIBRARIES -L${Ilmbase_LIB_DIR} -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} z pthread )
    ENDIF ()

    ADD_DEFINITIONS ( -DUSE_OPEN_EXR=1 )

    IF ( WIN32 )
        set ( OpenEXR_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../OpenEXR/include/OpenEXR )

        set( OpenEXR_LIB_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/lib)

        set( OpenEXR_LIBS
            IlmImf-2_4
            Half-2_4
            Imath-2_4
            Iex-2_4
            IexMath-2_4
            IlmThread-2_4
        )

        MESSAGE("Using MinGW threads")
        ADD_DEFINITIONS(-mthreads)
        set ( OpenEXR_LIBRARIES -Bstatic  -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} -L${MINGW_LIB_PATH} z mingwthrd winpthread -Bdynamic )

    ENDIF ()

    IF(APPLE)
        set ( OpenEXR_LIB_DIR /usr/local/Cellar/openexr/2.3.0/lib )
        set ( Ilmbase_LIB_DIR /usr/local/Cellar/ilmbase/2.3.0/lib )

        set ( OpenEXR_INCLUDE_DIRS /usr/local/Cellar/openexr/2.3.0/include/OpenEXR )
        set ( Ilmbase_INCLUDE_DIRS /usr/local/Cellar/ilmbase/2.3.0/include/OpenEXR )

        set( OpenEXR_LIBS
            IlmImf
            Half
            Imath
            Iex
            IexMath
            IlmThread
        )

        set( OpenEXR_LIBRARIES -L${Ilmbase_LIB_DIR} -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} z pthread )

        ADD_DEFINITIONS ( -pthread )

    ENDIF ()

ENDIF (USE_OPEN_EXR)

# SET ( CMAKE_CXX_FLAGS " -Wall -Wextra " )

# add_definitions ( -Wall )

include_directories (
    ${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIRS}
	${OpenEXR_INCLUDE_DIRS}
	${Ilmbase_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Script_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Xml_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	Fragmentarium/GUI
	SyntopiaCore/Logging
	Fragmentarium/Parser
	SyntopiaCore/Exceptions
	SyntopiaCore/Misc
	ThirdPartyCode)

set ( Fragmentarium_HDRS
	ThirdPartyCode/hdrloader.h
	ThirdPartyCode/asmbrowser.h
	ThirdPartyCode/EasingAnimation.h
	ThirdPartyCode/QtSpline.h
    ThirdPartyCode/Highlighter.h
    ThirdPartyCode/TimeLine.h
    ThirdPartyCode/VideoDialog.h
	SyntopiaCore/Exceptions/Exception.h
	SyntopiaCore/Logging/ListWidgetLogger.h
	SyntopiaCore/Logging/Logging.h
	SyntopiaCore/Misc/MiniParser.h
	SyntopiaCore/Misc/Misc.h
	SyntopiaCore/Misc/Version.h
	Fragmentarium/GUI/CameraControl.h
	Fragmentarium/GUI/FileManager.h
	Fragmentarium/Parser/Preprocessor.h
	)

set ( Fragmentarium_SRCS
	Fragmentarium/Main.cpp
	ThirdPartyCode/hdrloader.cpp
	ThirdPartyCode/asmbrowser.cpp
	Fragmentarium/GUI/CameraControl.cpp
	Fragmentarium/GUI/DisplayWidget.cpp
	ThirdPartyCode/EasingWindow.cpp
	Fragmentarium/GUI/FileManager.cpp
	Fragmentarium/GUI/TextEdit.cpp
	Fragmentarium/GUI/MainWindow.cpp
	Fragmentarium/GUI/OutputDialog.cpp
	ThirdPartyCode/QtSpline.cpp
	Fragmentarium/GUI/VariableEditor.cpp
	Fragmentarium/GUI/VariableWidget.cpp
    ThirdPartyCode/VideoDialog.cpp
	ThirdPartyCode/qrangeslider.cpp
	ThirdPartyCode/qdoublerangeslider.cpp
	Fragmentarium/Parser/Preprocessor.cpp
	SyntopiaCore/Logging/ListWidgetLogger.cpp
	SyntopiaCore/Logging/Logging.cpp
	SyntopiaCore/Misc/MiniParser.cpp
	SyntopiaCore/Misc/Misc.cpp
	SyntopiaCore/Misc/Version.cpp
    ThirdPartyCode/TimeLine.cpp
	)

set ( Fragmentarium_RSCS
	Fragmentarium.qrc
	)

QT5_ADD_RESOURCES(RSCS ${Fragmentarium_RSCS})

SET ( Fragmentarium_FORMS
        ThirdPartyCode/VideoDialog.ui
        ThirdPartyCode/TimeLineDialog.ui
        ThirdPartyCode/EasingDialog.ui
        Fragmentarium/GUI/OutputDialog.ui
        Fragmentarium/GUI/PreferencesDialog.ui
	)

QT5_WRAP_UI(UIS ${Fragmentarium_FORMS})

set ( Fragmentarium_MOCS
	Fragmentarium/GUI/PreferencesDialog.h
	)

QT5_WRAP_CPP(MOCS ${Fragmentarium_MOCS})

set(QM_FILES)
IF(Qt5LinguistTools_FOUND)
    OPTION(UPDATE_TRANSLATIONS ON)
    # prevent the generated files from being deleted during make clean
    set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

    add_custom_target(translations)
    file (GLOB TS_FILES Translations/*.ts)
    IF(UPDATE_TRANSLATIONS)
    foreach(_ts_file ${TS_FILES})

        # generate a sensible name for this translation file
        get_filename_component(_ts_name ${_ts_file} NAME_WE)

        # call lupdate ourselves
        add_custom_command(
            OUTPUT "${_ts_file}"
            DEPENDS ${Fragmentarium_HDRS} ${Fragmentarium_SRCS} ${UIS}
            COMMAND ${Qt5_LUPDATE_EXECUTABLE} -no-ui-lines -locations relative -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
        )

        # create a target that depends on the generated files
        add_custom_target(${_ts_name} DEPENDS ${_ts_file})

        # attach the custom target for this ts file to the parent target
        add_dependencies(translations ${_ts_name})
    endforeach()
    ENDIF(UPDATE_TRANSLATIONS)

    # generate qm files
    qt5_add_translation(QM_FILES ${TS_FILES})

ENDIF(Qt5LinguistTools_FOUND)

IF ( APPLE )
  add_executable ( ${PROJECT_NAME} MACOSX_BUNDLE ${QM_FILES} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ELSEIF ( UNIX )
  add_executable ( ${PROJECT_NAME} ${QM_FILES} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ELSEIF ( WIN32 )
#   set ( WIN32_EXECUTABLE ON )
  add_executable ( ${PROJECT_NAME} WIN32 ${QM_FILES} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
#   set ( WIN32_EXECUTABLE OFF )
  add_executable ( ${PROJECT_NAME}-console ${QM_FILES} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ENDIF ()

target_link_libraries ( ${PROJECT_NAME} ${OPENGL_LIBRARIES} ${OpenEXR_LIBRARIES} )
IF ( WIN32 )
target_link_libraries ( ${PROJECT_NAME}-console ${OPENGL_LIBRARIES} ${OpenEXR_LIBRARIES} )
ENDIF ()

qt5_use_modules(${PROJECT_NAME} OpenGL Widgets Gui Xml Script ScriptTools)
IF ( WIN32 )
qt5_use_modules(${PROJECT_NAME}-console OpenGL Widgets Gui Xml Script ScriptTools)
ENDIF ()

# Runtime support files
IF(APPLE)
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME} )
    # Installation
    INSTALL ( TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${DESTINATION_NAME}
    )
ELSEIF(UNIX)
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME} )
    # Installation
    INSTALL ( TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${DESTINATION_NAME}
    )
ELSEIF ( WIN32 )
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX} )
    # Installation
    INSTALL ( TARGETS
    ${PROJECT_NAME} ${PROJECT_NAME}-console
    DESTINATION
    ${DESTINATION_NAME}
    )
ENDIF()

INSTALL ( DIRECTORY
    Examples
    fqScript
    Misc
    DESTINATION
    ${DESTINATION_NAME}
    )

INSTALL ( FILES
    3Dickulus-HOW-TO.txt
    3Dickulus.changes
    LICENSE.GPL3
    LICENSE.LGPL
    LICENSE.README
    ../EULA.txt
    ../EULA_de.txt
    ../EULA_ru.txt
    ../EULA_nl.txt
DESTINATION
    ${DESTINATION_NAME}
)

INSTALL ( FILES ${QM_FILES}
DESTINATION
    ${DESTINATION_NAME}/Languages
)


IF(BUILD_LINUX_RPM OR BUILD_LINUX_DEB OR BUILD_WIN_ZIP)

    SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_BINARY_DIR}/../Notes.txt)
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An integrated environment for exploring GPU pixel graphics.")
    SET(PACKAGE_RELEASE "191225")

    IF(UNIX AND NOT APPLE)

        IF(BUILD_LINUX_RPM)
            SET(CMAKE_INSTALL_PREFIX "/usr/local")
            SET(CPACK_RPM_PACKAGE_NAME "Fragmentarium")
            SET(CPACK_RPM_PACKAGE_VERSION ${FRAGM_VERSION})
            SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
            SET(CPACK_RPM_PACKAGE_RELEASE ${PACKAGE_RELEASE})
            SET(CPACK_RPM_PACKAGE_LICENSE "LPGL,GPL")
            SET(CPACK_RPM_PACKAGE_GROUP "Graphics")
            SET(CPACK_RPM_PACKAGE_VENDOR "Digilantism")
            SET(CPACK_RPM_PACKAGE_RECOMMENDS "ffmpeg,mencoder")
            SET(CPACK_RPM_PACKAGE_PREDEPENDS "libilmbase12,libopenexr22")
            SET(CPACK_RPM_PACKAGE_DEPENDS "libQt5Core5, libQt5Gui5, libQt5OpenGL5, libQt5Script5, libQt5Widgets5, libQt5Xml5")
            SET(CPACK_RPM_PACKAGE_PROVIDES "FragM")
            SET(CPACK_GENERATOR "RPM")
            SET(CPACK_PACKAGE_FILE_NAME ${CPACK_RPM_PACKAGE_NAME}-${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${CMAKE_SYSTEM_NAME} )
        ENDIF()

        IF(BUILD_LINUX_DEB)
            SET(CMAKE_INSTALL_PREFIX "/usr/local")
            SET(CPACK_DEBIAN_PACKAGE_NAME "Fragmentarium")
            SET(CPACK_DEBIAN_PACKAGE_VERSION ${FRAGM_VERSION})
            SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
            SET(CPACK_DEBIAN_PACKAGE_RELEASE ${PACKAGE_RELEASE})
            SET(CPACK_DEBIAN_PACKAGE_LICENSE "lgpl,gpl")
            SET(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
            SET(CPACK_DEBIAN_PACKAGE_VENDOR "digilantism")
            SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ffmpeg,mencoder")
            SET(CPACK_DEBIAN_PACKAGE_PREDEPENDS "libilmbase12, libopenexr22")
            SET(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5, qtscript5, qtxml5")
            SET(CPACK_DEBIAN_PACKAGE_PROVIDES "FragM")
            SET(CPACK_GENERATOR "DEB")
            SET(CPACK_PACKAGE_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}-${CMAKE_SYSTEM_NAME} )
        ENDIF()

    ENDIF()

    SET(CPACK_PACKAGE_RELOCATABLE "YES")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_BINARY_DIR}/../LICENSE.README)
    SET(CPACK_PACKAGE_CONTACT "3dickulus@gmail.com")
    SET(CPACK_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build;.git*;")

    IF(WIN32)
            set(CPACK_GENERATOR "ZIP")
    ENDIF()

    INCLUDE(CPack)
ENDIF()
