cmake_minimum_required(VERSION 3.5)

project(Chip8Emu LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Thread)

add_executable(Chip8Emu
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  configdialog.cpp
  configdialog.h
  configdialog.ui
  keyboarddialog.cpp
  keyboarddialog.h
  keyboarddialog.ui
  keyboardmappingdialog.cpp
  keyboardmappingdialog.h
  keyboardmappingdialog.ui
  kbddevice.cpp
  kbddevice.h
  chip8keyboard.cpp
  chip8keyboard.h
  chip8.cpp
  chip8.h
  chip8display.cpp
  chip8display.h
  chip8pixelitem.cpp
  chip8pixelitem.h
  chip8graphicsview.cpp
  chip8graphicsview.h
)

target_link_libraries(Chip8Emu PRIVATE Qt5::Widgets pthread)
