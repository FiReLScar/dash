cmake_minimum_required(VERSION 3.16)
project(dash)

file(GLOB_RECURSE Sources Source/*.cpp)
include_directories(Include)
add_executable(dash ${Sources})
set_property(TARGET dash PROPERTY C_STANDARD 23)
target_link_libraries(dash PRIVATE link z curl)