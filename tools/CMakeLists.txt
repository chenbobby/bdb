add_executable(bdb bdb.cpp)

target_link_libraries(bdb PRIVATE bdb::libbdb)
