include(GNUInstallDirs)

add_library(libbdb libbdb.cpp)
add_library(bdb::libbdb ALIAS libbdb)

set_target_properties(
    libbdb
    PROPERTIES OUTPUT_NAME bdb
)

target_compile_features(libbdb PUBLIC cxx_std_17)

target_include_directories(
    libbdb
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/src/include
)

# Provide dependents with info about where `libbdb` will be installed.
install(TARGETS libbdb
    EXPORT bdb-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Allow dependents to install `bdb`'s public headers.
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Allow dependents to import `bdb` as a library.
install(
    EXPORT bdb-targets
    FILE bdb-config.cmake
    NAMESPACE bdb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bdb
)
