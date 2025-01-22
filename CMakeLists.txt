cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

set(CMAKE_DEBUG_POSTFIX d)

project(madina CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Source_Files
    madina.cpp
    madina.h
    shared.h
    shared.cpp)


find_package(Qt5 COMPONENTS Gui  REQUIRED)
find_package(visualmetafont REQUIRED)


add_library(${PROJECT_NAME} SHARED ${Source_Files})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(${PROJECT_NAME} Qt5::Gui visualmetafont)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
