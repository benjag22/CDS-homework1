cmake_minimum_required(VERSION 3.10)
project(CDS-homework1)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        CDS-homework1
        src/main.cpp
        src/wavelet_tree.hpp
        src/bit_vector.hpp
        src/bit_vector.hpp
        src/util.hpp
        src/fm_index.hpp
)
