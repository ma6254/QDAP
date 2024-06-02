cmake_minimum_required(VERSION 3.5)

project(QDAP LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES QDAP_zh_CN.ts)

add_subdirectory("yaml-cpp")

include_directories(
  libusb_hid_api/include
  src/DAP
  src/components
  src
)

if(ANDROID)
  add_library(QDAP SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${TS_FILES}
  )
else()
  add_executable(QDAP
    libusb_hid_api/include/hidapi.h
    libusb_hid_api/include/hidapi_winapi.h
    src/DAP/debug_cm.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/utils.cpp
    src/utils.h
    src/enum_writer_list.cpp
    src/enum_writer_list.h
    src/enum_writer_list.ui
    src/chip_selecter.cpp
    src/chip_selecter.h
    src/chip_selecter.ui
    src/components/hex_viewer.cpp
    src/components/hex_viewer.h
    src/components/hex_viewer.ui
    src/dap_hid.cpp
    src/dap_hid.h
    src/flash_algo.cpp
    src/flash_algo.h
    src/program_worker.cpp
    src/program_worker.h
    ${TS_FILES}
  )
endif()

target_link_libraries(
  QDAP
  PRIVATE Qt5::Widgets
  ${CMAKE_SOURCE_DIR}/libusb_hid_api/x64/hidapi.lib
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
