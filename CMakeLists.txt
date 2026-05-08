cmake_minimum_required(VERSION 3.16)
project(cds-hw1)

set(CMAKE_CXX_STANDARD 23)

add_executable(
    cds-hw1
    src/main.cpp
    src/wavelet_tree.hpp
    src/bit_vector.hpp
    src/fm_index.hpp
    src/util.hpp
    src/config.hpp
)

target_include_directories(cds-hw1 PRIVATE ${SDSL_INCLUDE_DIR})
target_link_directories(cds-hw1 PRIVATE ${SDSL_LIB_DIR})
target_link_libraries(cds-hw1 sdsl divsufsort divsufsort64)
