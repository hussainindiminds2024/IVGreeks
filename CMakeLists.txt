cmake_minimum_required(VERSION 3.14)

project(
    IVGREEKS
    VERSION 1.0
    LANGUAGES CXX
)

# ─────────────────────────────────────────────
#  C++ Standard
# ─────────────────────────────────────────────

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────────────────────────────────────────
#  Compiler Optimizations
# ─────────────────────────────────────────────

if(MSVC)

    add_compile_options(
        /O2
        /EHsc
    )

else()

    add_compile_options(
        -O3
        -march=native
        -ffast-math
    )

endif()

# ─────────────────────────────────────────────
#  Include Directories
# ─────────────────────────────────────────────

include_directories(include)

# ─────────────────────────────────────────────
#  Main Executable
# ─────────────────────────────────────────────

add_executable(
    ivgreeks
    app/main.cpp
)

add_executable(ivbench  app/test.cpp)
add_executable(ivlatencybench  app/latency_test.cpp)

# ─────────────────────────────────────────────
#  Linux / Unix math library
# ─────────────────────────────────────────────

if(UNIX)

    target_link_libraries(
        ivgreeks
        m
    )

endif()