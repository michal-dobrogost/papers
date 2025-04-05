cmake_minimum_required(VERSION 3.5)

project(csp-json)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

add_executable(cj-solve-gecode)
target_sources(cj-solve-gecode PRIVATE main.cpp cj/cj-csp.c cj/cj-csp-io.c)
target_link_libraries(cj-solve-gecode PUBLIC gecodekernel gecodesearch gecodeint gecodesupport)
install(TARGETS cj-solve-gecode DESTINATION .)
