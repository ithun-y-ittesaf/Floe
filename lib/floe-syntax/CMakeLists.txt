# floe-syntax — lexer, tokens, parser, AST
# Depends on: floe-diagnostics

add_library(floe-syntax STATIC
    src/token.cpp
    src/lexer.cpp
)

target_include_directories(floe-syntax PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(floe-syntax PUBLIC floe-diagnostics)

target_compile_features(floe-syntax PUBLIC cxx_std_20)
