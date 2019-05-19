project(hakaton)

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

set(pthreads)

add_executable (hakaton main.cpp)

find_package(jsoncpp REQUIRED)
include_directories(${jsoncpp_INCLUDE_DIRS})
link_libraries(${jsoncpp_LIBRARIES})
target_link_libraries(hakaton jsoncpp)

find_package(Boost 1.65.1 COMPONENTS system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(hakaton ${Boost_LIBRARIES})
endif()