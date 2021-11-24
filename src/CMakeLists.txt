cmake_minimum_required(VERSION 3.5)

project(ms2od VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UPDATE_TRANSLATIONS "Enable rescanning sources to update .ts files" OFF)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui WebEngineWidgets Widgets LinguistTools Charts REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui WebEngineWidgets Widgets LinguistTools LinguistTools Charts REQUIRED)

add_subdirectory(lang)

set(BLINKABLECB_SRC             blinkableCB.h blinkableCB.cpp)
set(CONVERT_SRC                 convert.h convert.cpp)
set(CRONOMETERLABEL_SRC         chronometerlabel.h chronometerlabel.cpp)
set(DOCKWIDGETCLOSEDETECT_SRC   dockwidgetclosedetect.h dockwidgetclosedetect.cpp)
set(FILEFINDER_SRC              filefinder.h filefinder.cpp)
set(FRAMECHART_SRC              framechart.h framechart.cpp framechart.ui)
set(LINEEDITCOLORED_SRC         lineeditcolored.h lineeditcolored.cpp)
set(LOGGER_SRC                  logger.h logger.cpp logger.tpp)
set(REMAININGTIMELABEL_SRC      remainingtimelabel.h remainingtimelabel.cpp)
set(UTILS_SRC                   utils.h utils.cpp utils.tpp)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        ${BLINKABLECB_SRC}
        ${CHRONOMETERLABEL_SRC}
        ${DOCKWIDGETCLOSEDETECT_SRC}
        ${FILEFINDER_SRC}
        ${FRAMECHART_SRC}
        ${LINEEDITCOLORED_SRC}
        ${LOGGER_SRC}
        ${REMAININGTIMELABEL_SRC}
        ${CONVERT_SRC}

        lang/lang.qrc
        icons/icons.qrc
        ${TS_FILES}
)



if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ms2od
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
)


    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(ms2od
        ${PROJECT_SOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

add_library(Utils SHARED ${UTILS_SRC})

target_link_libraries(Utils PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(ms2od PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::WebEngineCore
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts

    Utils
    )


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ms2od)
endif()
