add_executable(bdb bdb.cpp)

target_link_libraries(bdb PRIVATE bdb::libbdb PkgConfig::libedit)

include(GNUInstallDirs)

# Provide dependents with info about where `bdb` will be installed.
install(
    TARGETS bdb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
