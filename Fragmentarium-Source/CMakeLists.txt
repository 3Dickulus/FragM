project( Fragmentarium-2.0.b )
cmake_minimum_required( VERSION 2.8.12 )

cmake_policy(SET CMP0043 OLD)

set ( CMAKE_BUILD_TYPE Release )
set ( CMAKE_AUTOMOC ON )
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

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

# set to the mingw lib path
IF ( MINGW )
  set ( MINGW_LIB_PATH "C:\\Qt/Tools/mingw492_32/i686-w64-mingw32/lib" CACHE PATH "set to the mingw lib path" )
ENDIF ()

IF( NVIDIAGL4PLUS )
  ADD_DEFINITIONS ( -DNVIDIAGL4PLUS=1 )
ENDIF ()

IF(UNIX)
FIND_LIBRARY(OpenGL_LIBRARY GL )
ELSEIF(APPLE)
FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
ELSEIF(WIN32)
set(OpenGL_LIBRARY "-lopengl32" )
ENDIF ()

MARK_AS_ADVANCED (OpenGL_LIBRARY)

add_definitions ( -Wall )

IF ( USE_OPEN_EXR )
  set ( OpenEXR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/include/OpenEXR )
  ADD_DEFINITIONS ( -DUSE_OPEN_EXR=1 )
ENDIF ()

include_directories (
	${OpenEXR_INCLUDE_DIR}
	${Qt5Core_INCLUDE_DIRS} ${Qt5Script_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} 	${Qt5OpenGL_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
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

IF ( USE_OPEN_EXR )
set( OpenEXR_LIB_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/lib)

set( OpenEXR_LIBS
	IlmImf-2_1
	Half Imath-2_1
	Iex-2_1
	IexMath-2_1
	IlmThread-2_1
 )
ENDIF (USE_OPEN_EXR)

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


IF (UNIX)
  ADD_DEFINITIONS(-pthread)
  IF ( USE_OPEN_EXR )
    set(OpenEXR_LIBRARIES -Bstatic -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} -Bdynamic z pthread)
  ENDIF (USE_OPEN_EXR)
ENDIF(UNIX)

IF (MINGW)
    MESSAGE("Using MinGW threads")
    ADD_DEFINITIONS(-mthreads)
  IF ( USE_OPEN_EXR )
    set ( OpenEXR_LIBRARIES -Bstatic  -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} -L${MINGW_LIB_PATH} z mingwthrd winpthread -Bdynamic )
  ENDIF (USE_OPEN_EXR)
ENDIF(MINGW)

IF ( UNIX OR APPLE )
  add_executable ( Fragmentarium-2.0.b ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ELSEIF ( WIN32 )
  set ( WIN32_EXECUTABLE ON )
  add_executable ( Fragmentarium-2.0.b WIN32 ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ENDIF ()

target_link_libraries ( Fragmentarium-2.0.b ${OpenGL_LIBRARY} ${OpenEXR_LIBRARIES} )

qt5_use_modules(Fragmentarium-2.0.b OpenGL Widgets Gui Xml Script)

# Installation

INSTALL ( DIRECTORY
  Examples
  fqScript
  Misc
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}
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
  ${CMAKE_INSTALL_PREFIX}
)

INSTALL ( FILES
  ${QM}
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/Languages
)

INSTALL ( TARGETS
  Fragmentarium-2.0.b
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}
)
