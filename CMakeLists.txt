cmake_minimum_required(VERSION 3.5)

project(deep_tag LANGUAGES CXX)

include(ExternalProject)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_SYSTEM MATCHES "Windows")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/INSTALL_ROOT)
endif(CMAKE_SYSTEM MATCHES "Windows")

SET(CMAKE_INSTALL_RPATH "$ORIGIN/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}Multimedia REQUIRED)

if(CMAKE_SYSTEM MATCHES "Windows")
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/include)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/lib)
    set(OPENCV_LIBS opencv_core451d opencv_tracking451d opencv_video451d opencv_videoio451d opencv_imgcodecs451d opencv_imgproc451d)
  else(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/include)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/lib)
    set(OPENCV_LIBS opencv_core451 opencv_tracking451 opencv_video451 opencv_videoio451 opencv_imgcodecs451 opencv_imgproc451)
  endif(CMAKE_BUILD_TYPE MATCHES "Debug")
else(CMAKE_SYSTEM MATCHES "Windows")
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/debug/include/opencv4)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/debug/lib)
    set(OPENCV_LIBS opencv_core opencv_tracking opencv_video opencv_videoio opencv_imgcodecs opencv_imgproc)
  else(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OPENCV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/include/opencv4)
    set(OPENCV_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib)
    set(OPENCV_LIBS opencv_core opencv_tracking opencv_video opencv_videoio opencv_imgcodecs opencv_imgproc)
  endif(CMAKE_BUILD_TYPE MATCHES "Debug")
endif(CMAKE_SYSTEM MATCHES "Windows")

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

add_executable(deep_tag ${PROJECT_SOURCES} deep_tag.qrc)

target_include_directories(deep_tag PRIVATE ${OPENCV_INCLUDE_DIRS})
target_link_directories(deep_tag PRIVATE ${OPENCV_LIB_DIRS})
target_link_libraries(deep_tag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia ${OPENCV_LIBS})

if(CMAKE_SYSTEM MATCHES "Windows")
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    install(TARGETS deep_tag RUNTIME DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_core451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_tracking451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_video451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_videoio451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_imgcodecs451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_imgproc451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_dnn451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_calib3d451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_features2d451d.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/debug/x64/bin/opencv_flann451d.dll DESTINATION bin)
    install(CODE "execute_process(COMMAND windeployqt.exe ${CMAKE_CURRENT_BINARY_DIR}/INSTALL_ROOT/bin/deep_tag.exe)")
  else(CMAKE_BUILD_TYPE MATCHES "Debug")
    install(TARGETS deep_tag RUNTIME DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_core451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_tracking451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_video451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_videoio451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_imgcodecs451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_imgproc451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_dnn451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_calib3d451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_features2d451.dll DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/windows/vc16/release/x64/bin/opencv_flann451.dll DESTINATION bin)
    install(CODE "execute_process(COMMAND windeployqt.exe ${CMAKE_CURRENT_BINARY_DIR}/INSTALL_ROOT/bin/deep_tag.exe)")
  endif(CMAKE_BUILD_TYPE MATCHES "Debug")
else(CMAKE_SYSTEM MATCHES "Windows")
  install(TARGETS deep_tag RUNTIME DESTINATION deep_tag)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_core.so.4.5.1 DESTINATION deep_tag RENAME libopencv_core.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_tracking.so.4.5.1 DESTINATION deep_tag RENAME libopencv_tracking.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_video.so.4.5.1 DESTINATION deep_tag RENAME libopencv_video.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_videoio.so.4.5.1 DESTINATION deep_tag RENAME libopencv_videoio.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_imgcodecs.so.4.5.1 DESTINATION deep_tag RENAME libopencv_imgcodecs.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_imgproc.so.4.5.1 DESTINATION deep_tag RENAME libopencv_imgproc.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_dnn.so.4.5.1 DESTINATION deep_tag RENAME libopencv_dnn.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_calib3d.so.4.5.1 DESTINATION deep_tag RENAME libopencv_calib3d.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_features2d.so.4.5.1 DESTINATION deep_tag RENAME libopencv_features2d.so.4.5)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/opencv/linux/x64/release/lib/libopencv_flann.so.4.5.1 DESTINATION deep_tag RENAME libopencv_flann.so.4.5)
endif(CMAKE_SYSTEM MATCHES "Windows")
