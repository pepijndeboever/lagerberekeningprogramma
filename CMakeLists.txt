cmake_minimum_required(VERSION 3.0.0)
project(Lagerberekening VERSION 0.1.0)

#include(CTest)
#enable_testing()

#set(CMAKE_C_COMPILER icc)


# Build flags
if(UNIX)
add_compile_options(-Wall -Wextra -pedantic -march=native -std=c2x) # nog -O3 -Ofast toevoegen of -O2; -Og voor debuggen
endif(UNIX)
if(MSVC)
add_compile_definitions(WINNI)
endif(MSVC)

#set(CMAKE_EXE_LINKER_FLAGS -static)


add_executable(Lagerberekening src/main.c)

target_link_libraries(Lagerberekening lagerfuncties)

target_link_libraries(Lagerberekening raylib GL m pthread dl gsl gslcblas csv) # libm voor math.h

add_library(lagerfuncties src/lagerfuncties.c) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
