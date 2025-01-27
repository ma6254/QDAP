cmake_minimum_required(VERSION 3.5)

project(QDAP LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to
# qmake Android variables. Check http://doc.qt.io/qt-5/deployment-android.html
# for more information. They need to be set before the find_package(Qt5 ...)
# call.

# if(ANDROID) set(ANDROID_PACKAGE_SOURCE_DIR
# "${CMAKE_CURRENT_SOURCE_DIR}/android") if (ANDROID_ABI STREQUAL "armeabi-v7a")
# set(ANDROID_EXTRA_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
# ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so) endif() endif()
find_package(
  Qt5
  COMPONENTS
  Gui
  Widgets
  Network
  LinguistTools
  REQUIRED)

include_directories(${QT_INCLUDE_DIRS})

set(TS_FILES ${CMAKE_SOURCE_DIR} QDAP_zh_CN.ts)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

set(QM_FILES main.qrc)

find_program(GIT_EXECUTABLE git DOC "Git executable path")

if(GIT_EXECUTABLE)
  message(STATUS "Found Git executable at: ${GIT_EXECUTABLE}")
else()
  message(
    WARNING "Git executable not found. Submodule operations will be skipped.")
endif()

if(WIN32 AND GIT_EXECUTABLE)
  message(STATUS "Initializing and updating submodules on Windows...")

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_SUBMODULE_RESULT)

  if(NOT GIT_SUBMODULE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to initialize and update submodules.")
  endif()

# add_custom_target(update_submodules) add_custom_command( TARGET
# update_submodules POSTBUILD COMMAND "${GIT_EXECUTABLE}" submodule update
# --recursive WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
else()
  message(STATUS "Skipping submodule initialization and update.")
endif()

include_directories(
  src
  src/DAP
  src/config
  src/ui_components
  src/device_selector
  src/devices
  src/views/input_box_dialog
  src/views/chips_config_dialog
)

file(
  GLOB
  SRC_FILES
  src/*.cpp
  src/DAP/*.cpp
  src/config/*.cpp
  src/ui_components/*.cpp
  src/device_selector/*.cpp
  src/devices/*.cpp
  src/views/input_box_dialog/*.cpp
  src/views/chips_config_dialog/*.cpp
)

file(
  GLOB
  UI_FILES
  src/*.ui
  src/ui_components/*.ui
  src/device_selector/*.ui
  src/views/input_box_dialog/*.ui
  src/views/chips_config_dialog/*.ui
)

if(ANDROID)
  # ############################################################################
  # ANDROID
  # ############################################################################
  message(STATUS "Building for Android")
  add_library(QDAP SHARED ${SRC_FILES} ${UI_FILES} ${QM_FILES})
elseif(WIN32)
  # ############################################################################
  # Windows
  # ############################################################################
  message(STATUS "Building for Windows")

  # yaml-cpp
  add_subdirectory("vendor/yaml-cpp")
  set(yaml-cpp_LIBRARIES yaml-cpp)

  # QHexView
  add_subdirectory("vendor/QHexView")
  set(QHexView_LIBRARIES QHexView)


  include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    vendor/libusb_hid_api/include
    vendor/libusb/include
    vendor/yaml-cpp/include
    vendor/QHexView/include=
  )

  include_directories(ch341_ch347_lib/CH341PAR/LIB/CH347)

  add_executable(QDAP ${SRC_FILES} ${UI_FILES} ${QM_FILES})
  target_link_libraries(
    QDAP
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    # OpenSSL::SSL OpenSSL::Crypto
    ${CMAKE_SOURCE_DIR}/vendor/libusb_hid_api/x64/hidapi.lib
    ${CMAKE_SOURCE_DIR}/vendor/libusb/MinGW64/static/libusb-1.0.a
    yaml-cpp
    QHexView
  )
elseif(UNIX AND NOT APPLE)
  # ############################################################################
  # UNIX
  # ############################################################################
  message(STATUS "Building for Linux")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
  pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
  pkg_check_modules(LIBHIDAPI REQUIRED hidapi-hidraw)

  include_directories(
    ${LIBUSB_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIRS}
    ${LIBHIDAPI_LIBRARIES}
  )

  add_executable(QDAP ${SRC_FILES} ${UI_FILES} ${QM_FILES})
  target_link_libraries(
    QDAP PRIVATE Qt5::Widgets ${LIBUSB_LIBRARIES} ${YAML_CPP_LIBRARIES}
    ${LIBHIDAPI_LIBRARIES})

  target_include_directories(QDAP PRIVATE ${QT_INCLUDE_DIRS})

  # 添加 Linux 特定的安装规则
  install(TARGETS QDAP DESTINATION bin)
  install(DIRECTORY resources/ DESTINATION share/applications)

# install(FILES qdap.desktop DESTINATION share/applications) install(FILES
# qdap.png DESTINATION share/icons) install(FILES qdap.service DESTINATION
# etc/systemd/system) install(DIRECTORY contrib/icons/ DESTINATION
# share/icons/hicolor)
else()
  # ############################################################################
  # Unsupported
  # ############################################################################
  message(FATAL_ERROR "Unsupported platform")
endif()

add_definitions(-DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB)
