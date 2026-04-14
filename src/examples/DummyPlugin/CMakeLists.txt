project(DummyPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8.2 REQUIRED COMPONENTS Core Widgets)

add_library(DummyPlugin MODULE dummyplugin.cpp dummyplugin.h)

target_include_directories(DummyPlugin PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(DummyPlugin PRIVATE Qt6::Widgets cremniy)

set_target_properties(DummyPlugin PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
