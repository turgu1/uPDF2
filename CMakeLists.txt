cmake_minimum_required(VERSION 3.5)

project(uPDF2 VERSION 2.4.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Sql Svg Widgets Xml)

set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES /usr/local/lib ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})

configure_file(src/cmake_cfg.h.in cmake_cfg.h)
include_directories(${PROJECT_BINARY_BIN})

set(PROJECT_SOURCES
    src/bookmarksbrowser.cpp src/bookmarksbrowser.h
    src/bookmarksdb.cpp src/bookmarksdb.h
    src/bookmarkselector.cpp src/bookmarkselector.h
    src/config.cpp src/config.h
    src/documenteditdialog.cpp src/documenteditdialog.h
    src/documentmapperdelegate.cpp src/documentmapperdelegate.h
    src/documentmodel.cpp src/documentmodel.h
    src/documenttab.cpp src/documenttab.h
    src/entrymapperdelegate.cpp src/entrymapperdelegate.h
    src/filescache.cpp src/filescache.h
    src/loadpdffile.cpp src/loadpdffile.h
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/newbookmarkdialog.cpp src/newbookmarkdialog.h
    src/pagenbrdelegate.cpp src/pagenbrdelegate.h
    src/pdffile.cpp src/pdffile.h
    src/pdfloader.cpp src/pdfloader.h
    src/pdfpageworker.cpp src/pdfpageworker.h
    src/pdfviewer.cpp src/pdfviewer.h
    src/preferencesdialog.cpp src/preferencesdialog.h
    src/selectrecentdialog.cpp src/selectrecentdialog.h
    src/updf.h
    src/utils.cpp src/utils.h
    ui/bookmarksbrowser.ui
    ui/bookmarkselector.ui
    ui/documenteditdialog.ui
    ui/mainwindow.ui
    ui/newbookmarkdialog.ui
    ui/preferencesdialog.ui
    ui/selectrecentdialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(uPDF2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET uPDF2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(uPDF2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(uPDF2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(uPDF2 PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    poppler
)

target_include_directories(uPDF2 PRIVATE
    /usr/include/poppler
    /usr/local/include
    /usr/local/include/poppler
)

set_target_properties(uPDF2 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(updf_resource_files
    "updf_resources/img/updf-128x128.png"
    "updf_resources/img/updf-256x256.png"
    "updf_resources/img/updf-48x48.png"
    "updf_resources/img/updf-512x512.png"
    "updf_resources/img/updf-64x64.png"
    "updf_resources/svg/16x16/arrow-left.svg"
    "updf_resources/svg/24x24/arrow-left.svg"
    "updf_resources/svg/24x24/book.svg"
    "updf_resources/svg/24x24/bookmark.svg"
    "updf_resources/svg/24x24/chevron-down.svg"
    "updf_resources/svg/24x24/chevron-up.svg"
    "updf_resources/svg/24x24/chevrons-down.svg"
    "updf_resources/svg/24x24/chevrons-up.svg"
    "updf_resources/svg/24x24/file-text.svg"
    "updf_resources/svg/24x24/folder.svg"
    "updf_resources/svg/24x24/maximize-2.svg"
    "updf_resources/svg/24x24/minimize-2.svg"
    "updf_resources/svg/24x24/minus-square.svg"
    "updf_resources/svg/24x24/plus-square.svg"
    "updf_resources/svg/24x24/power.svg"
    "updf_resources/svg/24x24/search.svg"
    "updf_resources/svg/24x24/settings.svg"
    "updf_resources/svg/24x24/zoom-in.svg"
    "updf_resources/svg/24x24/zoom-out.svg"
)

qt_add_resources(uPDF2 "updf_icons.qrc"
    PREFIX
        "/icons"
    BASE
        "updf_resources"
    FILES
        ${updf_resource_files}
)

set(updf1_resource_files
    "updf_resources/img/busy.gif"
)

qt_add_resources(uPDF2 "updf_agif.qrc"
    PREFIX
        "/agif"
    BASE
        "updf_resources"
    FILES
        ${updf1_resource_files}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(uPDF2)
endif()

#-------------------

if(false)
cmake_minimum_required(VERSION 3.16)
project(uPDF2 VERSION 2.3.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Sql Svg Widgets Xml)

qt_standard_project_setup()

qt_add_executable(uPDF2
    src/bookmarksbrowser.cpp src/bookmarksbrowser.h
    src/bookmarksdb.cpp src/bookmarksdb.h
    src/bookmarkselector.cpp src/bookmarkselector.h
    src/config.cpp src/config.h
    src/documenteditdialog.cpp src/documenteditdialog.h
    src/documentmapperdelegate.cpp src/documentmapperdelegate.h
    src/documentmodel.cpp src/documentmodel.h
    src/documenttab.cpp src/documenttab.h
    src/entrymapperdelegate.cpp src/entrymapperdelegate.h
    src/filescache.cpp src/filescache.h
    src/loadpdffile.cpp src/loadpdffile.h
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/newbookmarkdialog.cpp src/newbookmarkdialog.h
    src/pagenbrdelegate.cpp src/pagenbrdelegate.h
    src/pdffile.cpp src/pdffile.h
    src/pdfloader.cpp src/pdfloader.h
    src/pdfpageworker.cpp src/pdfpageworker.h
    src/pdfviewer.cpp src/pdfviewer.h
    src/preferencesdialog.cpp src/preferencesdialog.h
    src/selectrecentdialog.cpp src/selectrecentdialog.h
    src/updf.h
    src/utils.cpp src/utils.h
    ui/bookmarksbrowser.ui
    ui/bookmarkselector.ui
    ui/documenteditdialog.ui
    ui/mainwindow.ui
    ui/newbookmarkdialog.ui
    ui/preferencesdialog.ui
    ui/selectrecentdialog.ui
)

target_include_directories(uPDF2 PRIVATE
    /usr/include/poppler
    /usr/local/include
    /usr/local/include/poppler
)

target_compile_definitions(uPDF2 PRIVATE
    APP_VERSION="2.3.0"
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(uPDF2 PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Sql
    Qt::Svg
    Qt::Widgets
    Qt::Xml
    poppler.a
    -L/usr/local/lib
)

target_compile_options(uPDF2
    -Wall
    -Wextra
    -Wno-unused-parameter
    -isystem
)

# Resources:
set(updf_resource_files
    "updf_resources/img/updf-128x128.png"
    "updf_resources/img/updf-256x256.png"
    "updf_resources/img/updf-48x48.png"
    "updf_resources/img/updf-512x512.png"
    "updf_resources/img/updf-64x64.png"
    "updf_resources/svg/16x16/arrow-left.svg"
    "updf_resources/svg/24x24/arrow-left.svg"
    "updf_resources/svg/24x24/book.svg"
    "updf_resources/svg/24x24/bookmark.svg"
    "updf_resources/svg/24x24/chevron-down.svg"
    "updf_resources/svg/24x24/chevron-up.svg"
    "updf_resources/svg/24x24/chevrons-down.svg"
    "updf_resources/svg/24x24/chevrons-up.svg"
    "updf_resources/svg/24x24/file-text.svg"
    "updf_resources/svg/24x24/folder.svg"
    "updf_resources/svg/24x24/maximize-2.svg"
    "updf_resources/svg/24x24/minimize-2.svg"
    "updf_resources/svg/24x24/minus-square.svg"
    "updf_resources/svg/24x24/plus-square.svg"
    "updf_resources/svg/24x24/power.svg"
    "updf_resources/svg/24x24/search.svg"
    "updf_resources/svg/24x24/settings.svg"
    "updf_resources/svg/24x24/zoom-in.svg"
    "updf_resources/svg/24x24/zoom-out.svg"
)

qt_add_resources(uPDF2 "updf_icons.qrc"
    PREFIX
        "/icons"
    BASE
        "updf_resources"
    FILES
        ${updf_resource_files}
)
set(updf1_resource_files
    "updf_resources/img/busy.gif"
)

qt_add_resources(uPDF2 "updf_agif.qrc"
    PREFIX
        "/agif"
    BASE
        "updf_resources"
    FILES
        ${updf1_resource_files}
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(uPDF2 PRIVATE
        Qt::Widgets
    )
endif()

install(TARGETS uPDF2
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET uPDF2
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
endif()
