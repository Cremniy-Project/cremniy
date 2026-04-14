project(PluginSystem LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8.2 REQUIRED COMPONENTS Core Widgets)

add_library(PluginSystem STATIC PluginManager.cpp PluginManager.h IPlugin.h)

target_include_directories(PluginSystem PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PluginSystem PRIVATE Qt6::Core Qt6::Widgets)
