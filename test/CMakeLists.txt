add_executable(ismctest main.cpp game.cpp)
target_link_libraries(ismctest ismcsolver)
target_include_directories(ismctest PRIVATE ismcsolver)
