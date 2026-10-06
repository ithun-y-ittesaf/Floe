# floe-diagnostics — source spans, error types, rendering
# This module has ZERO dependencies on other floe libraries.

add_library(floe-diagnostics STATIC
    src/source.cpp
    src/span.cpp
    src/diagnostic.cpp
)

target_include_directories(floe-diagnostics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(floe-diagnostics PUBLIC cxx_std_20)
