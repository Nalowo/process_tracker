cmake_minimum_required(VERSION 3.27.1)

project(process_tracker CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(proc_t src/main.cpp)
# add_executable(test src/test.cpp)
# target_link_libraries(test PRIVATE pdh.lib) // подключение счетчиков