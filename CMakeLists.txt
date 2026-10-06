cmake_minimum_required(VERSION 3.22)
project(Floe
    VERSION 0.1.0
    DESCRIPTION "A high-level language for describing media-processing pipelines"
    LANGUAGES CXX
)

# ── Global settings ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put all binaries in a predictable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Options ──────────────────────────────────────────────────────────────────
option(FLOE_BUILD_TESTS "Build unit tests" ON)

# ── Dependencies ─────────────────────────────────────────────────────────────
if(FLOE_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    # Prevent gtest from overriding our compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# ── Libraries (dependency order) ─────────────────────────────────────────────
# Each subdirectory defines a static library target: floe-<name>
add_subdirectory(lib/floe-diagnostics)   # error types, spans, rendering
add_subdirectory(lib/floe-syntax)        # lexer, tokens, parser, AST
# add_subdirectory(lib/floe-sema)        # M4: name resolution, type checker
# add_subdirectory(lib/floe-ir)          # M2: media IR graph
# add_subdirectory(lib/floe-eval)        # M2: AST → IR evaluator
# add_subdirectory(lib/floe-ffmpeg)      # M3: FFmpeg backend

# ── CLI executable ───────────────────────────────────────────────────────────
# add_subdirectory(tools/floe-cli)       # M1: `floe` binary

# ── Tests ────────────────────────────────────────────────────────────────────
if(FLOE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
