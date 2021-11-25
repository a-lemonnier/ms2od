cmake_minimum_required(VERSION 3.5)

project(ms2od VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UPDATE_TRANSLATIONS "Enable rescanning sources to update .ts files" OFF)

set(LANG_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/lang)
set(BCK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.bck)

file(GLOB LANG_FILES ${LANG_DIR}/*.ts)
file(GLOB LANG_FILES_BCK ${BCK_DIR}/*.ts)

if(EXISTS "${LANG_FILES}")
    message(STATUS "TS files exist: backing up...")
    file(MAKE_DIRECTORY  ${BCK_DIR})
    file(COPY ${LANG_FILES} DESTINATION  ${BCK_DIR})
else()
    message(STATUS "TS files do not exist.")
    if(EXISTS "${LANG_FILES_BCK}")
            message(STATUS "Restoring TS files...")
            file(COPY ${LANG_FILES_BCK} DESTINATION  ${LANG_DIR})
    else()
            message(STATUS "Cannot restore TS files: skipping.")
    endif()
endif()

add_subdirectory(src)
