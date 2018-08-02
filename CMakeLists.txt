cmake_minimum_required (VERSION 3.1.0)
 
project(NerDisco)

#-------------------------------------------------------------------------------
#set up Qt5

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Widgets finds its own dependencies (Core and Gui).
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)
# find_package(Qt5Multimedia REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Xml REQUIRED)

#-------------------------------------------------------------------------------
#set up compiler flags and executable names

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    #set up compiler flags for Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2") #optimize
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 5.4)
            message(SEND_ERROR "You need at least G++ 4.7 to compile ${PROJECT_NAME}!")
    endif()
    #set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2") #optimize
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()


if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()

#-------------------------------------------------------------------------------
#set up RtMidi

if(MSVC)
    add_definitions(-D__WINDOWS_MM__)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    find_package (Threads)
    find_package (ALSA)
    add_definitions(-D__LINUX_ALSA__)
endif()

#-------------------------------------------------------------------------------
#define basic sources and headers

set(TARGET_HEADERS
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioConversion.h
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioInterface.h
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioProcessing.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/CodeEdit.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ColorOperations.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Deck.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/DisplayImageConverter.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/DisplayThread.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLCompileThread.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/I_MIDIControl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageOperations.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/LiveView.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIDeviceInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIParameterConnection.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIParameterMapping.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIWorker.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeBase.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeEnum.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeQString.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeRanged.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ParameterQtConnect.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parameters.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ParameterScanlineDirection.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ParameterT.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/QAspectRatioLabel.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/QTextEditLineNumberArea.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/QTextEditStatusArea.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/QtMIDIButton.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/QtSpinBoxAction.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/SignalJoiner.h
#	${CMAKE_CURRENT_SOURCE_DIR}/src/SwapThread.h
	${CMAKE_CURRENT_SOURCE_DIR}/rtmidi/RtMidi.h
)

set(TARGET_SOURCES
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioConversion.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioInterface.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioProcessing.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CodeEdit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Deck.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DisplayImageConverter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DisplayThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLSLCompileThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageOperations.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/LiveView.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIDeviceInterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIInterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIParameterConnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIParameterMapping.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MIDIWorker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/NerDisco.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeBase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeEnum.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeQString.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/NodeRanged.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ParameterQtConnect.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ParameterScanlineDirection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/QAspectRatioLabel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/QTextEditLineNumberArea.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/QTextEditStatusArea.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/QtMIDIButton.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/QtSpinBoxAction.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/SignalJoiner.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/src/SwapThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rtmidi/RtMidi.cpp
)

set(TARGET_FORMS
	${CMAKE_CURRENT_SOURCE_DIR}/src/Deck.ui
	${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.ui
)
qt5_wrap_ui(FORMS_ADDED ${TARGET_FORMS})

set(TARGET_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc
)
qt5_add_resources(RESOURCE_ADDED ${TARGET_RESOURCES})

#Add source directory to include directories for building .ui files etc.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

#-------------------------------------------------------------------------------
#set up build directories

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

#-------------------------------------------------------------------------------
#define target

add_executable(${PROJECT_NAME} ${TARGET_SOURCES} ${TARGET_HEADERS} ${RESOURCE_ADDED} ${FORMS_ADDED})
target_link_libraries(${PROJECT_NAME} ${Qt5Widgets_LIBRARIES} Qt5::OpenGL Qt5::SerialPort Qt5::Xml)

#add libraries for RtMidi
if(MSVC)
    target_link_libraries (${PROJECT_NAME} winmm ${QT_QTMAIN_LIBRARY})
	set(CMAKE_CREATE_WIN32_EXE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${ALSA_LIBRARY})
endif()

