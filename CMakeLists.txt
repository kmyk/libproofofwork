cmake_minimum_required (VERSION 3.1)
project (proofofwork)

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mtune=native -march=native")

# check AVX
include(CheckCSourceCompiles)
check_c_source_compiles ("#include <immintrin.h>\n int main(void) { __m256i ymm = _mm256_setzero_si256(); return 0; }" HAVE_AVX)
if (NOT HAVE_AVX)
    message (FATAL_ERROR "AVX instructions are not supported in your CPU")
endif ()

# check OpenMP
find_package (OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif ()

include_directories (include)
add_library (proofofwork SHARED
    src/md5-avx2.c)
