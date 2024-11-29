add_library(libbdb libbdb.cpp)
add_library(bdb::libbdb ALIAS libbdb)

set_target_properties(
    libbdb
    PROPERTIES OUTPUT_NAME bdb
)

target_compile_features(libbdb PUBLIC cxx_std_17)

target_include_directories(
    libbdb
    PRIVATE include
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)
