cmake_minimum_required( VERSION 3.20 )
cmake_policy(SET CMP0043 NEW)
IF(UNIX)
cmake_policy(SET CMP0071 NEW)
ELSE ()
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0048 NEW)
ENDIF ()

set(FRAGM_MAJOR_VERSION "2")
set(FRAGM_MINOR_VERSION "5")
set(FRAGM_REVISION "7")

set(FRAGM_VERSION "${FRAGM_MAJOR_VERSION}.${FRAGM_MINOR_VERSION}.${FRAGM_REVISION}")

string( TIMESTAMP PACKAGE_BUILD "%y%m%d" UTC )

ADD_DEFINITIONS (
    -DFRAGM_VERSION=${FRAGM_VERSION}
    -DFRAGM_MAJOR_VERSION=${FRAGM_MAJOR_VERSION}
    -DFRAGM_MINOR_VERSION=${FRAGM_MINOR_VERSION}
    -DFRAGM_REVISION=${FRAGM_REVISION}
    -DPACKAGE_BUILD=${PACKAGE_BUILD}
    )

message( STATUS "Fragmentarium-${FRAGM_VERSION}-${PACKAGE_BUILD}" )

project(Fragmentarium-${FRAGM_VERSION} VERSION ${FRAGM_VERSION} LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

IF(UNIX AND NOT APPLE)

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


set(OpenGL_GL_PREFERENCE "LEGACY")
option (GL_PREFERENCE_GLVND "Open GL preference GLVND" OFF)
MARK_AS_ADVANCED (GL_PREFERENCE_GLVND)
IF ( GL_PREFERENCE_GLVND )
    set(OpenGL_GL_PREFERENCE "GLVND")
ENDIF ()

IF ( NOT MINGW )
    find_package ( glm REQUIRED )
ENDIF( NOT MINGW )

find_package ( OpenGL REQUIRED )

find_package ( Qt5LinguistTools REQUIRED )

find_package ( Qt5 COMPONENTS Core OpenGL Widgets Gui Xml Script ScriptTools REQUIRED )

# ON if you want opengl 4+ capabilities
OPTION ( USE_OPENGL_4 "Build with OpenGL 4.1" ON)
MARK_AS_ADVANCED (USE_OPENGL_4)
IF ( USE_OPENGL_4 )
    ADD_DEFINITIONS ( -DUSE_OPENGL_4=1 )
ENDIF ()

ADD_DEFINITIONS ( -DQT_DEVICE_PIXEL_RATIO=1 -Wall )

# ON if you have OpenEXR libs and include files
OPTION ( USE_OPEN_EXR "Build with OpenEXR" ON)
IF(APPLE)
    SET(USE_OPEN_EXR OFF)
ENDIF ()
IF ( USE_OPEN_EXR )
    ADD_DEFINITIONS ( -DUSE_OPEN_EXR=1 )
ENDIF ()

OPTION ( BUILD_SOURCE_ZIP "Build ZIP sources package" OFF)
OPTION ( BUILD_WIN_ZIP "Build Windows zip package" OFF)
OPTION ( BUILD_LINUX_DEB "Build Linux Debian package" OFF)
OPTION ( BUILD_LINUX_RPM "Build Linux RPM package" OFF)

# set to the lib path for mingw
IF ( MINGW )
    get_filename_component(MINGWBINPATH ${CMAKE_CXX_COMPILER} PATH)
    set(MINGW_LIB_PATH ${MINGWBINPATH}/../lib CACHE FILEPATH DOCSTRING "Path to MinGW import libraries")
    set(MINGW_INC_PATH ${MINGWBINPATH}/../include CACHE FILEPATH DOCSTRING "Path to MinGW include headers")

    set(OPENGL_LIBRARIES -L${MINGW_LIB_PATH} -lopengl32 )
    set ( GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../ )
ENDIF ()

MARK_AS_ADVANCED (OPENGL_LIBRARIES)

set (OpenEXTRA_LIBRARIES "")
set (OpenEXR_INCLUDE_DIRS "")

IF ( APPLE  )
    set ( GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../glm )
ENDIF ()

MARK_AS_ADVANCED (OpenEXTRA_LIBRARIES)
MARK_AS_ADVANCED (OpenEXR_INCLUDE_DIRS)
MARK_AS_ADVANCED (GLM_INCLUDE_DIRS)

IF ( USE_OPEN_EXR )

    IF(UNIX AND NOT APPLE)
        find_package ( OpenEXR REQUIRED )
        set( OpenEXTRA_LIBRARIES z pthread )
    ENDIF (UNIX AND NOT APPLE)

    IF ( MINGW )
        set ( ZLIB_INCLUDE_DIR ${MINGW_LIB_PATH} )
        set ( ZLIB_LIBRARY ${MINGW_LIB_PATH}/libz.a )

        set ( Imath_DIR ${CMAKE_SOURCE_DIR}/../Imath/lib/cmake/Imath )
        set ( OpenEXR_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/lib/cmake/OpenEXR )
        
        list(APPEND CMAKE_MODULE_PATH "${Imath_DIR}/../")
        list(APPEND CMAKE_MODULE_PATH "${OpenEXR_DIR}/../")
        
        find_package ( Imath REQUIRED )
        find_package ( OpenEXR REQUIRED )

        #this is some fudge as OpenEXR uses #include <half.h> instead of #include <Imath/half.h>
        set ( OpenEXR_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/../OpenEXR/include
        ${CMAKE_SOURCE_DIR}/../Imath/include
        ${CMAKE_SOURCE_DIR}/../Imath/include/Imath)

        set( OpenEXR_LIB_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/lib )

        set( OpenEXR_LIBS
            Iex-3_2
            IlmThread-3_2
            OpenEXR-3_2
            OpenEXRCore-3_2
            OpenEXRUtil-3_2
        )

        MESSAGE("Considered ... ${Imath_CONSIDERED_CONFIGS} ${Imath_CONSIDERED_VERSIONS}" )
        MESSAGE("Considered ... ${OpenEXR_CONSIDERED_CONFIGS} ${OpenEXR_CONSIDERED_VERSIONS}")
        
        MESSAGE("Using MinGW threads")
        ADD_DEFINITIONS(-mthreads)
        set ( OpenEXTRA_LIBRARIES -Bstatic -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} -L${MINGW_LIB_PATH} z mingwthrd winpthread -Bdynamic )
    
    ENDIF ( MINGW )

    IF(APPLE)
        set( OpenEXTRA_LIBRARIES z pthread )
        ADD_DEFINITIONS ( -pthread )
    ENDIF(APPLE)

ENDIF (USE_OPEN_EXR)

# SET ( CMAKE_CXX_FLAGS " -Wall -Wextra " )
# add_definitions ( -Wall )

set(CMAKE_CXX_FLAGS, " -Wl --stack 100000000 -Wl --heap 100000000 ")

include_directories (
    ${OPENGL_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${OpenEXR_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Script_INCLUDE_DIRS}
    ${Qt5ScriptTools_INCLUDE_DIRS}
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
    ThirdPartyCode
    ThirdPartyCode/qtgradienteditor)

set ( Fragmentarium_HDRS
    Fragmentarium/GUI/CameraControl.h
    Fragmentarium/GUI/TextEdit.h
    Fragmentarium/GUI/VariableEditor.h
    Fragmentarium/GUI/VariableWidget.h
    Fragmentarium/GUI/FloatWidget.h
    Fragmentarium/GUI/IntWidget.h
    Fragmentarium/GUI/ColorWidget.h
    Fragmentarium/GUI/SamplerWidget.h
    Fragmentarium/GUI/BoolWidget.h
    Fragmentarium/GUI/DisplayWidget.h
    Fragmentarium/GUI/PreferencesDialog.h
    Fragmentarium/GUI/OutputDialog.h
    Fragmentarium/GUI/FileManager.h
    Fragmentarium/GUI/MainWindow.h
    Fragmentarium/Parser/Preprocessor.h
    SyntopiaCore/Exceptions/Exception.h
    SyntopiaCore/Logging/ListWidgetLogger.h
    SyntopiaCore/Logging/Logging.h
    SyntopiaCore/Misc/MiniParser.h
    SyntopiaCore/Misc/Misc.h
    SyntopiaCore/Misc/Version.h
    ThirdPartyCode/EasingAnimation.h
    ThirdPartyCode/EasingWindow.h
    ThirdPartyCode/QtSpline.h
    ThirdPartyCode/VideoDialog.h
    ThirdPartyCode/asmbrowser.h
    ThirdPartyCode/hdrloader.h
    ThirdPartyCode/qdoublerangeslider.h
    ThirdPartyCode/qrangeslider.h
    ThirdPartyCode/ExpSpinBox.h
    ThirdPartyCode/Highlighter.h
    ThirdPartyCode/TimeLine.h
    ThirdPartyCode/ggr2glsl.h
    ThirdPartyCode/qtgradienteditor/qtgradientwidget.h
    ThirdPartyCode/qtgradienteditor/qtgradientviewdialog.h
    ThirdPartyCode/qtgradienteditor/qtgradientview.h
    ThirdPartyCode/qtgradienteditor/qtgradientutils.h
    ThirdPartyCode/qtgradienteditor/qtgradientstopswidget.h
    ThirdPartyCode/qtgradienteditor/qtgradientstopsmodel.h
    ThirdPartyCode/qtgradienteditor/qtgradientstopscontroller.h
    ThirdPartyCode/qtgradienteditor/qtgradientmanager.h
    ThirdPartyCode/qtgradienteditor/qtgradienteditor.h
    ThirdPartyCode/qtgradienteditor/qtgradientdialog.h
    ThirdPartyCode/qtgradienteditor/qtcolorline.h
    ThirdPartyCode/qtgradienteditor/qtcolorbutton.h
    )

set ( Fragmentarium_SRCS
    Fragmentarium/GUI/OutputDialog.cpp
    Fragmentarium/GUI/FileManager.cpp
    Fragmentarium/GUI/CameraControl.cpp
    Fragmentarium/GUI/MainWindow.cpp
    Fragmentarium/GUI/TextEdit.cpp
    Fragmentarium/GUI/VariableEditor.cpp
    Fragmentarium/GUI/VariableWidget.cpp
    Fragmentarium/GUI/FloatWidget.cpp
    Fragmentarium/GUI/IntWidget.cpp
    Fragmentarium/GUI/ColorWidget.cpp
    Fragmentarium/GUI/SamplerWidget.cpp
    Fragmentarium/GUI/BoolWidget.cpp
    Fragmentarium/GUI/DisplayWidget.cpp
    Fragmentarium/Parser/Preprocessor.cpp
    Fragmentarium/Main.cpp
    SyntopiaCore/Logging/ListWidgetLogger.cpp
    SyntopiaCore/Logging/Logging.cpp
    SyntopiaCore/Misc/MiniParser.cpp
    SyntopiaCore/Misc/Misc.cpp
    SyntopiaCore/Misc/Version.cpp
    ThirdPartyCode/QtSpline.cpp
    ThirdPartyCode/VideoDialog.cpp
    ThirdPartyCode/asmbrowser.cpp
    ThirdPartyCode/hdrloader.cpp
    ThirdPartyCode/qdoublerangeslider.cpp
    ThirdPartyCode/qrangeslider.cpp
    ThirdPartyCode/EasingWindow.cpp
    ThirdPartyCode/ExpSpinBox.cpp
    ThirdPartyCode/TimeLine.cpp
    ThirdPartyCode/ggr2glsl.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientwidget.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientviewdialog.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientview.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientutils.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientstopswidget.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientstopsmodel.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientstopscontroller.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientmanager.cpp
    ThirdPartyCode/qtgradienteditor/qtgradienteditor.cpp
    ThirdPartyCode/qtgradienteditor/qtgradientdialog.cpp
    ThirdPartyCode/qtgradienteditor/qtcolorline.cpp
    ThirdPartyCode/qtgradienteditor/qtcolorbutton.cpp
    )

set ( Fragmentarium_RSCS
    Fragmentarium.qrc
    ThirdPartyCode/qtgradienteditor/qtgradienteditor.qrc
    )

QT5_ADD_RESOURCES(RSCS ${Fragmentarium_RSCS})

SET ( Fragmentarium_FORMS
    ThirdPartyCode/VideoDialog.ui
    ThirdPartyCode/TimeLineDialog.ui
    ThirdPartyCode/EasingDialog.ui
    ThirdPartyCode/qtgradienteditor/qtgradienteditor.ui
    ThirdPartyCode/qtgradienteditor/qtgradientdialog.ui
    ThirdPartyCode/qtgradienteditor/qtgradientview.ui
    ThirdPartyCode/qtgradienteditor/qtgradientviewdialog.ui
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
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -no-obsolete -no-ui-lines -locations relative -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
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
ELSEIF ( MINGW )
  add_executable ( ${PROJECT_NAME} WIN32 ${QM_FILES} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ENDIF ()

target_link_libraries ( ${PROJECT_NAME}
${OPENGL_LIBRARIES}
    # For OpenEXR/Imath 3.x:
      $<$<TARGET_EXISTS:OpenEXR::OpenEXR>:OpenEXR::OpenEXR>
      $<$<TARGET_EXISTS:Imath::Imath>:Imath::Imath>
      $<$<TARGET_EXISTS:Imath::Half>:Imath::Half>
    # For OpenEXR 2.4/2.5:
      $<$<TARGET_EXISTS:OpenEXR::IlmImf>:OpenEXR::IlmImf>
      $<$<TARGET_EXISTS:IlmBase::Imath>:IlmBase::Imath>
      $<$<TARGET_EXISTS:IlmBase::Half>:IlmBase::Half>
      $<$<TARGET_EXISTS:IlmBase::IlmThread>:IlmBase::IlmThread>
      $<$<TARGET_EXISTS:IlmBase::Iex>:IlmBase::Iex>
      $<$<TARGET_EXISTS:IlmBase::IexMath>:IlmBase::IexMath>
${OpenEXTRA_LIBRARIES} )

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
ELSEIF ( MINGW )
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX} )
    # Installation
    INSTALL ( TARGETS
    ${PROJECT_NAME}
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


IF(BUILD_LINUX_RPM OR BUILD_LINUX_DEB OR BUILD_WIN_ZIP OR BUILD_SOURCE_ZIP)

    SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_BINARY_DIR}/../Notes.txt)
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An integrated environment for exploring GPU pixel graphics.")
    SET(CPACK_PACKAGE_CONTACT "3dickulus@gmail.com")
    IF(UNIX AND NOT APPLE)

        IF(BUILD_LINUX_RPM)
            SET(CMAKE_INSTALL_PREFIX "/usr/local")
            SET(CPACK_RPM_PACKAGE_NAME "Fragmentarium")
            SET(CPACK_RPM_PACKAGE_VERSION ${FRAGM_VERSION})
            SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
            SET(CPACK_RPM_PACKAGE_RELEASE ${PACKAGE_BUILD})
            SET(CPACK_RPM_PACKAGE_LICENSE "LPGL,GPL")
            SET(CPACK_RPM_PACKAGE_GROUP "Graphics")
            SET(CPACK_RPM_PACKAGE_VENDOR "Digilantism")
            SET(CPACK_RPM_PACKAGE_RECOMMENDS "ffmpeg,mencoder")
            SET(CPACK_RPM_PACKAGE_PREDEPENDS "libopenexr25")
            SET(CPACK_RPM_PACKAGE_DEPENDS "libQt5OpenGL5, libQt5Gui5, libQt5Script5, libQt5ScriptTools5, libQt5Xml5")
            SET(CPACK_RPM_PACKAGE_PROVIDES "Fragmentarium")
            SET(CPACK_GENERATOR "RPM")
            SET(CPACK_PACKAGE_FILE_NAME ${CPACK_RPM_PACKAGE_NAME}-${FRAGM_VERSION}-${PACKAGE_BUILD}-${CMAKE_SYSTEM_NAME} )
        ENDIF(BUILD_LINUX_RPM)

        IF(BUILD_LINUX_DEB)
            SET(CMAKE_INSTALL_PREFIX "/usr/local")
            SET(CPACK_DEBIAN_PACKAGE_NAME "fragmentarium")
            SET(CPACK_DEBIAN_PACKAGE_VERSION ${FRAGM_VERSION})
            SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
            SET(CPACK_DEBIAN_PACKAGE_RELEASE ${PACKAGE_BUILD})
            SET(CPACK_DEBIAN_PACKAGE_LICENSE "lgpl,gpl")
            SET(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
            SET(CPACK_DEBIAN_PACKAGE_VENDOR "digilantism")
            SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ffmpeg,mencoder")
            SET(CPACK_DEBIAN_PACKAGE_PREDEPENDS "libopenexr25")
            SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5opengl5, libqt5gui5, libqt5script5, libqt5scripttools5, libqt5xml5")
            SET(CPACK_DEBIAN_PACKAGE_PROVIDES "fragmentarium")
            SET(CPACK_GENERATOR "DEB")
            SET(CPACK_PACKAGE_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}-${FRAGM_VERSION}-${PACKAGE_BUILD}-${CMAKE_SYSTEM_NAME} )
        ENDIF(BUILD_LINUX_DEB)

    ENDIF(UNIX AND NOT APPLE)

    IF(MINGW)
        set(CPACK_GENERATOR "ZIP")
    ENDIF()

    IF(BUILD_SOURCE_ZIP)
        SET(CPACK_PACKAGE_NAME "Fragmentarium")
        SET(CPACK_PACKAGE_VERSION ${FRAGM_VERSION}-${PACKAGE_BUILD})
        SET(CPACK_PACKAGE_RELOCATABLE "YES")
        SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_BINARY_DIR}/../LICENSE.README)
        SET(CPACK_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build*;.git*;")
        set(CPACK_SOURCE_GENERATOR "ZIP")
        set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build*;.git*;.*~$")
        set(CPACK_VERBATIM_VARIABLES YES)
    ENDIF()

    INCLUDE(CPack)
ENDIF()
