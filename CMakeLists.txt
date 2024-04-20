cmake_minimum_required(VERSION 3.27.1)

project(process_tracker CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(proc_t src/main.cpp)
target_link_libraries(proc_t PRIVATE ws2_32 PSAPI ${CONAN_LIBS})

# add_executable(test src/test.cpp)
# target_link_libraries(test PRIVATE pdh.lib) # подключение счетчиков