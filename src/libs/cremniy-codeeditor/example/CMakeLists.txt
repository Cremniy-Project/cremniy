cmake_minimum_required(VERSION 3.6)
project(CodeEditorExample)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets)

add_executable(CodeEditorExample
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
)

target_link_libraries(CodeEditorExample
    Qt6::Widgets
    cremniy-codeeditor
)
