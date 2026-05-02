cmake_minimum_required(VERSION 3.16)
project(CDS-homework1)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        CDS-homework1
        src/main.cpp
        src/wavelet_tree.hpp
        src/bit_vector.hpp
        src/util.hpp
        src/fm_index.hpp
)

target_include_directories(CDS-homework1 PRIVATE ${SDSL_INCLUDE_DIR})
target_link_directories(CDS-homework1 PRIVATE ${SDSL_LIB_DIR})
target_link_libraries(CDS-homework1 sdsl divsufsort)