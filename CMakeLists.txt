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
ExternalProject_Add(OpenCV
  PREFIX OpenCV
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv-master"
  CMAKE_CACHE_ARGS
        -DOPENCV_EXTRA_MODULES_PATH:PATH=${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv_contrib-master/modules
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/OPENCV_INSTALL
)

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
#find_package(OpenCV REQUIRED HINTS ${CMAKE_CURRENT_BINARY_DIR}/OPENCV_INSTALL)
#find_package(OpenCV REQUIRED HINTS ${PROJECT_SOURCE_DIR}/opencv)

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

add_dependencies(deep_tag OpenCV)

# opencv/include/opencv4
#target_include_directories(deep_tag PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(deep_tag PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/OPENCV_INSTALL/include/opencv4)
target_link_directories(deep_tag PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/OPENCV_INSTALL/lib)
#target_link_libraries(deep_tag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia ${OpenCV_LIBS})
target_link_libraries(deep_tag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia opencv_core opencv_imgproc opencv_tracking opencv_video opencv_videoio opencv_imgcodecs)
