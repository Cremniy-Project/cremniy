cmake_minimum_required(VERSION 3.16)

project(cremniy VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
)

add_subdirectory(components/codeeditor)
add_subdirectory(components/hexview)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED
        ${PROJECT_SOURCES}
        
        app/IDEWindow/idewindow.cpp
        app/IDEWindow/idewindow.h

        app/WelcomeWindow/welcomeform.cpp
        app/WelcomeWindow/welcomeform.h

        dialogs/filecreatedialog.cpp
        dialogs/filecreatedialog.h

        widgets/clickablelineedit.cpp
        widgets/clickablelineedit.h
        widgets/filestabwidget.cpp
        widgets/filestabwidget.h
        widgets/filetab.cpp
        widgets/filetab.h
        widgets/filetreeview.cpp
        widgets/filetreeview.h
        widgets/tooltabwidget.cpp
        widgets/tooltabwidget.h

        utils/iconprovider.cpp
        utils/iconprovider.h

        resources/cremniy_res.qrc

        widgets/codeeditortab.cpp widgets/codeeditortab.h
        widgets/hexviewtab.cpp widgets/hexviewtab.h
        widgets/disassemblertab.cpp widgets/disassemblertab.h

        include/toolwidget.hpp
        include/tooltab.h

        utils/utils.cpp
        utils/utils.h
        utils/globalwidgetsmanager.h
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        
        app/IDEWindow/idewindow.cpp
        app/IDEWindow/idewindow.h

        app/WelcomeWindow/welcomeform.cpp
        app/WelcomeWindow/welcomeform.h

        dialogs/filecreatedialog.cpp
        dialogs/filecreatedialog.h

        widgets/clickablelineedit.cpp
        widgets/clickablelineedit.h
        widgets/filestabwidget.cpp
        widgets/filestabwidget.h
        widgets/filetab.cpp
        widgets/filetab.h
        widgets/filetreeview.cpp
        widgets/filetreeview.h
        widgets/tooltabwidget.cpp
        widgets/tooltabwidget.h

        utils/iconprovider.cpp
        utils/iconprovider.h

        resources/cremniy_res.qrc

        widgets/codeeditortab.cpp widgets/codeeditortab.h
        widgets/hexviewtab.cpp widgets/hexviewtab.h
        widgets/disassemblertab.cpp widgets/disassemblertab.h

        include/toolwidget.hpp
        include/tooltab.h

        utils/utils.cpp
        utils/utils.h
        utils/globalwidgetsmanager.h
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    codeeditor
    hexview
    Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)