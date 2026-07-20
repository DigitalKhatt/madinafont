cmake_minimum_required(VERSION 3.24)

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
if(NOT TARGET digitalkhatt::otlayout)
  find_package(digitalkhatt_otlayout REQUIRED CONFIG)
endif()

set(FONT_TARGET_NAME "${PROJECT_NAME}")
if(DEFINED DIGITALKHATT_FONT_TARGET_NAME)
  set(FONT_TARGET_NAME "${DIGITALKHATT_FONT_TARGET_NAME}")
endif()

add_library(${FONT_TARGET_NAME} SHARED ${Source_Files})

set_target_properties(${FONT_TARGET_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(${FONT_TARGET_NAME} PRIVATE Qt5::Gui digitalkhatt::otlayout)

install(TARGETS ${FONT_TARGET_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
