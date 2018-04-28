cmake_minimum_required (VERSION 2.4)
project (proofofwork)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -O3 -mtune=native -march=native")

# check SIMD instructions
include(CheckCSourceCompiles)
check_c_source_compiles ("#include <immintrin.h>\n int main(void) { __m256i ymm = _mm256_setzero_si256(); return 0; }" HAVE_AVX)
check_c_source_compiles ("#include <immintrin.h>\n int main(void) { __m256i ymm = _mm256_setzero_si256(); ymm = _mm256_abs_epi32(ymm); return 0; }" HAVE_AVX2)
if (NOT HAVE_AVX2)
    message (FATAL_ERROR "AVX2 instructions are not supported in your CPU")
endif ()

# check OpenMP
find_package (OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif ()

# check OpenSSL
find_package(OpenSSL REQUIRED)

include_directories (include)
set (sources
    src/md5-avx2.c
    src/sha1-avx2.c
    src/sha256-openssl.c
    src/common.c
)
add_library (proofofwork-shared SHARED ${sources})
add_library (proofofwork-static STATIC ${sources})
set_target_properties (proofofwork-shared PROPERTIES OUTPUT_NAME proofofwork)
set_target_properties (proofofwork-static PROPERTIES OUTPUT_NAME proofofwork)
target_link_libraries(proofofwork-shared ${OPENSSL_LIBRARIES})
target_link_libraries(proofofwork-static ${OPENSSL_LIBRARIES})
