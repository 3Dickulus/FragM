project( Fragmentarium-2.0.b )
cmake_minimum_required( VERSION 2.8.12 )

cmake_policy(SET CMP0043 OLD)

set ( CMAKE_BUILD_TYPE Release )
set ( CMAKE_AUTOMOC ON )
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

find_package ( OpenGL REQUIRED )
find_package ( Qt5Core REQUIRED )
find_package ( Qt5OpenGL REQUIRED )
find_package ( Qt5Widgets REQUIRED )
find_package ( Qt5Gui REQUIRED )
find_package ( Qt5Xml REQUIRED )
find_package ( Qt5Script REQUIRED )
find_package ( Qt5LinguistTools REQUIRED )


SET ( CMAKE_CXX_FLAGS "-std=c++11" )

# ON to compile with AsmBrowser enabled requires nVidia + GL > 4.0
OPTION ( NVIDIAGL4PLUS "Build for nVidia GL v4+" ON)
# ON if you have OpenEXR libs and include files
OPTION ( USE_OPEN_EXR "Build with OpenEXR" OFF)

set (OpenEXR_LIBRARIES "")

IF( NVIDIAGL4PLUS )
  ADD_DEFINITIONS ( -DNVIDIAGL4PLUS=1 )
ENDIF ()


IF ( USE_OPEN_EXR )

    FIND_PATH(OpenEXR_LIB_DIR NAMES lib64/libIlmImf.so)
    FIND_PATH(OpenEXR_INCLUDE_DIR NAMES OpenEXR/half.h )

    if(OpenEXR_LIB_DIR AND OpenEXR_INCLUDE_DIR)

        set( OpenEXR_LIBS
            IlmImf
            Half
            Imath
            Iex
            IexMath
            IlmThread
        )
        
        set ( OpenEXR_INCLUDE_DIR ${OpenEXR_INCLUDE_DIR}/OpenEXR )
        set ( OpenEXR_LIB_DIR ${OpenEXR_LIB_DIR}/lib64 )

        ADD_DEFINITIONS ( -DUSE_OPEN_EXR=1 -pthread )

        set(OpenEXR_LIBRARIES -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} z pthread)

    endif(OpenEXR_LIB_DIR AND OpenEXR_INCLUDE_DIR)

ENDIF (USE_OPEN_EXR)

add_definitions ( -Wall )

include_directories (
    ${OPENGL_INCLUDE_DIR}
	${OpenEXR_INCLUDE_DIR}
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

set(QM)
IF(Qt5LinguistTools_FOUND)
    IF(NOT Qt5_LUPDATE_EXECUTABLE)
        MESSAGE(WARNING "Qt's lupdate not found.")
    ENDIF()
    IF(NOT Qt5_LRELEASE_EXECUTABLE)
        MESSAGE(WARNING "Qt's lrelease not found.")
    ENDIF()
    # Translation files
    IF(Qt5_LUPDATE_EXECUTABLE AND Qt5_LRELEASE_EXECUTABLE)
        option (UPDATE_TRANSLATIONS "Update *.ts translation files from source files." OFF)
        if (UPDATE_TRANSLATIONS)
          set(FILES_TO_TRANSLATE ${Fragmentarium_HDRS} ${Fragmentarium_SRCS} ${UIS})
          qt5_create_translation(QM ${FILES_TO_TRANSLATE} OPTIONS -no-obsolete -no-ui-lines)
        else ()
          MESSAGE("Creating *.qm from *.ts language files...")
          file (GLOB TRANSLATIONS *.ts)
          qt5_add_translation(QM ${TRANSLATIONS})
        endif () 
        #add_custom_target (translations_target DEPENDS ${QM})
    ENDIF()
    # prevent the generated files from being deleted during make clean
    set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

ENDIF(Qt5LinguistTools_FOUND)

add_executable ( Fragmentarium-2.0.b ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})

target_link_libraries ( Fragmentarium-2.0.b ${OPENGL_LIBRARIES} ${OpenEXR_LIBRARIES} )

qt5_use_modules(Fragmentarium-2.0.b OpenGL Widgets Gui Xml Script)

# Installation

INSTALL ( DIRECTORY
  Examples
  fqScript
  Misc
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/Fragmentarium
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
  ${CMAKE_INSTALL_PREFIX}/Fragmentarium
)

INSTALL ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_de.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_en.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_nl.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_ru.qm
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/Fragmentarium/Languages
)

INSTALL ( TARGETS
  Fragmentarium-2.0.b
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/Fragmentarium
)

SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_BINARY_DIR}/../Notes.txt)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An integrated environment for exploring GPU pixel graphics.")
SET(CPACK_PACKAGE_NAME "Fragmentarium")
SET(CPACK_PACKAGE_VERSION "2.0.0")
SET(CPACK_RPM_PACKAGE_RELEASE "1")
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
SET(CPACK_RPM_PACKAGE_RELEASE "1")
SET(CPACK_RPM_PACKAGE_LICENSE "LPGL,GPL")
SET(CPACK_RPM_PACKAGE_GROUP "Graphics")
SET(CPACK_RPM_PACKAGE_VENDOR "Digilantism")
SET(CPACK_RPM_PACKAGE_REQUIRES "openexr(x86-64) >= 2.1.0, libQt5OpenGL5(x86-64) >= 5.6.1")
SET(CPACK_RPM_SOURCE_PACKAGE_REQUIRES "OpenEXR-devel >= 2.1.0, libQt5OpenGL5-devel >= 5.6.1")
SET(CPACK_PACKAGE_RELOCATABLE "YES")
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_BINARY_DIR}/../LICENSE.README)

SET(CPACK_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build;.git*;")
SET(CPACK_GENERATOR "RPM")
SET(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build;.git*;")
SET(CPACK_SOURCE_GENERATOR "TBZ2")

INCLUDE(CPack)
 
