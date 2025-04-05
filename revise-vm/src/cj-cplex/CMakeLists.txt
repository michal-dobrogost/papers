cmake_minimum_required(VERSION 3.5)

project(csp-json-cplex)

#find_package(Cplex)
set(CPLEX_LIBS concert cp cplex iljs ilocplex opl)
set(CPLEX_LIBS_DIRS "/Applications/CPLEX_Studio_Community2211/opl/lib/arm64_osx/static_pic")
set(CPLEX_INCLUDE_DIRS "/Applications/CPLEX_Studio_Community2211/opl/include")

add_executable(cj-solve-cplex)
target_sources(cj-solve-cplex PRIVATE main.cpp cj/cj-csp.c cj/cj-csp-io.c)
target_include_directories(cj-solve-cplex AFTER PUBLIC ../)
target_link_directories(cj-solve-cplex PUBLIC ${CPLEX_LIBS_DIRS})
target_include_directories(cj-solve-cplex AFTER PUBLIC ${CPLEX_INCLUDE_DIRS})
target_link_libraries(cj-solve-cplex PUBLIC ${CPLEX_LIBS})
install(TARGETS cj-solve-cplex DESTINATION .)
