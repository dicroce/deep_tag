cmake_minimum_required(VERSION 3.5)

project(deep_tag LANGUAGES CXX)

include(ExternalProject)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# try add GIT_REPOSITORY and GIT_TAG
#ExternalProject_Add(OpenCV
#  PREFIX OpenCV
#  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv-master"
#  CMAKE_CACHE_ARGS
#        -DOPENCV_EXTRA_MODULES_PATH:PATH=${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv_contrib-master/modules
#        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/OPENCV_INSTALL
#)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}Multimedia REQUIRED)

if(WIN32)
  if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/include)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/lib)
    set(OPENCV_LIBS opencv_core451d opencv_tracking451d opencv_video451d opencv_videoio451d opencv_imgcodecs451d opencv_imgproc451d)
  else(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/include)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/lib)
    set(OPENCV_LIBS opencv_core451 opencv_tracking451 opencv_video451 opencv_videoio451 opencv_imgcodecs451 opencv_imgproc451)
  endif(CMAKE_BUILD_TYPE EQUAL "DEBUG")
else(WIN32)
  if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/debug/include/opencv4)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/debug/lib)
    set(OPENCV_LIBS opencv_core451d opencv_tracking opencv_video opencv_videoio opencv_imgcodecs opencv_imgproc)
  else(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/include/opencv4)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib)
    set(OPENCV_LIBS opencv_core opencv_tracking opencv_video opencv_videoio opencv_imgcodecs opencv_imgproc)
  endif(CMAKE_BUILD_TYPE EQUAL "DEBUG")
endif(WIN32)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        createrectdialog.cpp
        createrectdialog.h
        createrectdialog.ui
        startupdialog.cpp
        startupdialog.h
        startupdialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(deep_tag
        ${PROJECT_SOURCES}
        deep_tag.qrc
    )
else()
    if(ANDROID)
        add_library(deep_tag SHARED
            ${PROJECT_SOURCES}
            deep_tag.qrc
        )
    else()
        add_executable(deep_tag
            ${PROJECT_SOURCES}
            deep_tag.qrc
        )
    endif()
endif()

target_include_directories(deep_tag PRIVATE ${OPENCV_INCLUDE_DIRS})
target_link_directories(deep_tag PRIVATE ${OPENCV_LIB_DIRS})
target_link_libraries(deep_tag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia ${OPENCV_LIBS})
