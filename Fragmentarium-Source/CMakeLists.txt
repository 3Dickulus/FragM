project( Fragmentarium-2.0.b )
cmake_minimum_required( VERSION 2.8.12 )

cmake_policy(SET CMP0043 OLD)

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
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

find_package ( OpenGL REQUIRED )
find_package ( Qt5Core REQUIRED )
find_package ( Qt5OpenGL REQUIRED )
find_package ( Qt5Widgets REQUIRED )
find_package ( Qt5Gui REQUIRED )
find_package ( Qt5Xml REQUIRED )
find_package ( Qt5Script REQUIRED )
find_package ( Qt5LinguistTools )


# ON to compile with AsmBrowser enabled requires nVidia + GL > 4.0
OPTION ( NVIDIAGL4PLUS "Build for nVidia GL v4+" ON)
# ON if you have OpenEXR libs and include files
OPTION ( USE_OPEN_EXR "Build with OpenEXR" OFF)

OPTION ( BUILD_WIN_ZIP "Build Windows zip package" OFF)
OPTION ( BUILD_LINUX_DEB "Build Linux Debian package" OFF)
OPTION ( BUILD_LINUX_RPM "Build Linux RPM package" OFF)

# set to the mingw lib path
IF ( MINGW )
  set ( MINGW_LIB_PATH "C:\\Qt/Tools/mingw492_32/i686-w64-mingw32/lib" CACHE PATH "set to the mingw lib path" )
ENDIF ()

IF( NVIDIAGL4PLUS )
  ADD_DEFINITIONS ( -DNVIDIAGL4PLUS=1 )
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

MARK_AS_ADVANCED (OpenEXR_LIBRARIES)
MARK_AS_ADVANCED (OpenEXR_INCLUDE_DIRS)
MARK_AS_ADVANCED (Ilmbase_INCLUDE_DIRS)

IF ( USE_OPEN_EXR )

    IF(UNIX AND NOT APPLE)
        find_package ( OpenEXR REQUIRED )
    ENDIF ()

    ADD_DEFINITIONS ( -DUSE_OPEN_EXR=1 )

    IF ( MINGW )
        set ( OpenEXR_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../OpenEXR/include/OpenEXR )
    
        set( OpenEXR_LIB_DIR ${CMAKE_SOURCE_DIR}/../OpenEXR/lib)

        set( OpenEXR_LIBS
            IlmImf-2_1
            Half Imath-2_1
            Iex-2_1
            IexMath-2_1
            IlmThread-2_1
        )

        MESSAGE("Using MinGW threads")
        ADD_DEFINITIONS(-mthreads)
        set ( OpenEXR_LIBRARIES -Bstatic  -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} -L${MINGW_LIB_PATH} z mingwthrd winpthread -Bdynamic )

    ENDIF ()

    IF(APPLE)
#       FIND_PATH(OpenEXR_INCLUDE_DIRS NAMES OpenEXR/ImfVersion.h )
#       FIND_PATH(Ilmbase_INCLUDE_DIRS NAMES OpenEXR/half.h )

#         FIND_PATH(OpenEXR_LIB_DIR NAMES lib/libIlmImf.dylib)
#         FIND_PATH(Ilmbase_LIB_DIR NAMES lib/libHalf.dylib)

        set ( OpenEXR_LIB_DIR /usr/local/Cellar/openexr/2.2.0/lib )
        set ( Ilmbase_LIB_DIR /usr/local/Cellar/ilmbase/2.2.0/lib )

        set ( OpenEXR_INCLUDE_DIRS /usr/local/Cellar/openexr/2.2.0/include/OpenEXR )
        set ( Ilmbase_INCLUDE_DIRS /usr/local/Cellar/ilmbase/2.2.0/include/OpenEXR )

        set( OpenEXR_LIBS
            IlmImf
            Half
            Imath
            Iex
            IexMath
            IlmThread
        )
        
        set(OpenEXR_LIBRARIES -Bstatic -L${Ilmbase_LIB_DIR} -L${OpenEXR_LIB_DIR} ${OpenEXR_LIBS} z pthread -Bdynamic)

        ADD_DEFINITIONS ( -pthread )

    ENDIF ()
  
ENDIF (USE_OPEN_EXR)

    IF(APPLE)
        SET ( CMAKE_CXX_FLAGS "-std=c++11 -Wno-deprecated-register -Wno-unused-result" )
    ELSE ()
        SET ( CMAKE_CXX_FLAGS "-std=c++11 -Wno-unused-result" )
    ENDIF ()

add_definitions ( -Wall )

include_directories (
    ${OPENGL_INCLUDE_DIR}
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
          MESSAGE("Creating language files...")
          set(FILES_TO_TRANSLATE ${Fragmentarium_HDRS} ${Fragmentarium_SRCS} ${UIS})
          qt5_create_translation(QM ${FILES_TO_TRANSLATE} OPTIONS -no-obsolete -no-ui-lines)
        else ()
          MESSAGE("Adding language files...")
          file (GLOB TRANSLATIONS Translations/*.ts)
          qt5_add_translation(QM ${TRANSLATIONS})
        endif () 
        #add_custom_target (translations_target DEPENDS ${QM})
    ENDIF()
    # prevent the generated files from being deleted during make clean
    set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)

ENDIF(Qt5LinguistTools_FOUND)

IF ( APPLE )
  add_executable ( ${PROJECT_NAME} MACOSX_BUNDLE ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ELSEIF ( UNIX )
  add_executable ( ${PROJECT_NAME} ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ELSEIF ( WIN32 )
  set ( WIN32_EXECUTABLE ON )
  add_executable ( ${PROJECT_NAME} WIN32 ${QM} ${Fragmentarium_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
ENDIF ()

target_link_libraries ( ${PROJECT_NAME} ${OPENGL_LIBRARIES} ${OpenEXR_LIBRARIES} )

qt5_use_modules(${PROJECT_NAME} OpenGL Widgets Gui Xml Script)

# Installation
INSTALL ( TARGETS
  ${PROJECT_NAME}
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
)

# Runtime support files
IF(APPLE)
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/${PROJECT_NAME}.app/Contents/MacOS )
ELSE()
    set( DESTINATION_NAME ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME} )
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

INSTALL ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_de.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_en.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_nl.qm
    ${CMAKE_CURRENT_BINARY_DIR}/Fragmentarium_ru.qm
DESTINATION
    ${DESTINATION_NAME}/Languages
)


IF(BUILD_LINUX_RPM OR BUILD_LINUX_DEB OR BUILD_WIN_ZIP)

    SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_BINARY_DIR}/../Notes.txt)
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An integrated environment for exploring GPU pixel graphics.")

    IF(UNIX)

    IF(BUILD_LINUX_RPM)
        SET(CPACK_RPM_PACKAGE_NAME "Fragmentarium")
        SET(CPACK_RPM_PACKAGE_VERSION "2.0.0")
        SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
        SET(CPACK_RPM_PACKAGE_RELEASE "1")
        SET(CPACK_RPM_PACKAGE_LICENSE "LPGL,GPL")
        SET(CPACK_RPM_PACKAGE_GROUP "Graphics")
        SET(CPACK_RPM_PACKAGE_VENDOR "Digilantism")
        SET(CPACK_RPM_PACKAGE_REQUIRES "openexr(x86-64) >= 2.1.0, libQt5OpenGL5(x86-64) >= 5.6.1")
        SET(CPACK_GENERATOR "RPM")
    ENDIF()

    IF(BUILD_LINUX_DEB)
        SET(CPACK_DEB_PACKAGE_NAME "Fragmentarium")
        SET(CPACK_DEB_PACKAGE_VERSION "2.0.0")
        SET(CPACK_DEB_PACKAGE_ARCHITECTURE "x86_64")
        SET(CPACK_DEB_PACKAGE_RELEASE "1")
        SET(CPACK_DEB_PACKAGE_LICENSE "LPGL,GPL")
        SET(CPACK_DEB_PACKAGE_GROUP "Graphics")
        SET(CPACK_DEB_PACKAGE_VENDOR "Digilantism")
        SET(CPACK_DEB_PACKAGE_REQUIRES "openexr(x86-64) >= 2.1.0, libQt5OpenGL5(x86-64) >= 5.7.1")
        SET(CPACK_GENERATOR "DEB")
    ENDIF()

    SET(CPACK_TBZ2_SOURCE_PACKAGE_REQUIRES "OpenEXR-devel >= 2.1.0, libQt5OpenGL5-devel >= 5.4.1")

    ENDIF()

    SET(CPACK_PACKAGE_RELOCATABLE "YES")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_BINARY_DIR}/../LICENSE.README)
    SET(CPACK_PACKAGE_CONTACT "3dickulus@gmail.com")
    SET(CPACK_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build;.git*;")

    IF(WIN32)
            set(CPACK_GENERATOR "ZIP")
    ENDIF()


    SET(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;/.bzr/;/.hg/;/.git/;.swp\$;.#;/#;.kdev4;.directory;build;.git*;")
    SET(CPACK_SOURCE_GENERATOR "TBZ2")

    INCLUDE(CPack)
ENDIF()
