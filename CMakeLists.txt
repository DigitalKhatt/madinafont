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


if(NOT TARGET digitalkhatt::otlayout)
  find_package(digitalkhatt_otlayout REQUIRED CONFIG)
endif()

set(FONT_TARGET_NAME "${PROJECT_NAME}")
if(DEFINED DIGITALKHATT_FONT_TARGET_NAME)
  set(FONT_TARGET_NAME "${DIGITALKHATT_FONT_TARGET_NAME}")
endif()

if(EMSCRIPTEN)
  # Built as an Emscripten "side module": a relocatable .wasm dynamically
  # loaded at runtime by OtLayout.cpp's dlopen() call (see global.h's
  # SLPREFIX/SLEXT), mirroring the native dlopen()-based plugin architecture
  # instead of statically linking. SIDE_MODULE codegen is decided at compile
  # time as well as link time, so the flag is needed on both.
  add_executable(${FONT_TARGET_NAME} ${Source_Files})

  set_target_properties(${FONT_TARGET_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    PREFIX "lib"
    SUFFIX ".wasm")

  # Headers only -- never link digitalkhatt_otlayout's compiled code here.
  # It's STATIC under EMSCRIPTEN, so linking it normally would bake a second,
  # independent copy of OtLayout/GlyphVis/Lookup/Subtable into this side
  # module, breaking Madina's bidirectional calls into the main module's
  # live objects.
  target_include_directories(${FONT_TARGET_NAME} PRIVATE
    $<TARGET_PROPERTY:digitalkhatt::otlayout,INTERFACE_INCLUDE_DIRECTORIES>)

  # glaze::glaze is header-only (INTERFACE), so linking it normally is safe
  # (no compiled code to duplicate) -- needed to pick up its cxx_std_23
  # requirement, which digitalkhatt_otlayout also inherits transitively via
  # a normal target_link_libraries and which some headers pulled in via
  # ParameterJson.h/GlazeJson.h require (std::expected, std::unreachable,
  # std::byteswap).
  target_link_libraries(${FONT_TARGET_NAME} PRIVATE glaze::glaze)

  target_compile_options(${FONT_TARGET_NAME} PRIVATE "SHELL:-s SIDE_MODULE=1")
  target_link_options(${FONT_TARGET_NAME} PRIVATE "SHELL:-s SIDE_MODULE=1")

  # Must match VisualMetaFontWasm's exception-handling ABI (see
  # digitalkhattwasm/CMakeLists.txt) -- a mismatch here otherwise fails at
  # side-module instantiation with a LinkError on a missing invoke_*
  # import, since Madina's indirect calls through OtLayout/MPFont/GlyphVis
  # virtual interfaces need the same invoke_ trampoline ABI as the main
  # module compiled with DISABLE_EXCEPTION_CATCHING=0.
  target_compile_options(${FONT_TARGET_NAME} PRIVATE "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")
  target_link_options(${FONT_TARGET_NAME} PRIVATE "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")

  add_custom_command(TARGET ${FONT_TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${FONT_TARGET_NAME}>" "${CMAKE_CURRENT_SOURCE_DIR}/../digitalkhatt.org/ClientApp/src/assets"
    COMMENT "Copying to output directory")
else()
  add_library(${FONT_TARGET_NAME} SHARED ${Source_Files})

  set_target_properties(${FONT_TARGET_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

  target_link_libraries(${FONT_TARGET_NAME} PRIVATE digitalkhatt::otlayout)

  install(TARGETS ${FONT_TARGET_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
endif()
