cmake_minimum_required(VERSION 3.5)

project(csp-json-or-tools)

find_package(absl REQUIRED)

add_executable(cj-solve-or-tools-cp)
target_sources(cj-solve-or-tools-cp PRIVATE main-cp.cpp cj/cj-csp.c cj/cj-csp-io.c)
target_link_libraries(cj-solve-or-tools-cp PUBLIC ortools absl::flat_hash_map)
target_include_directories(cj-solve-or-tools-cp AFTER PUBLIC ../)
install(TARGETS cj-solve-or-tools-cp DESTINATION .)

add_executable(cj-solve-or-tools-cpsat)
target_sources(cj-solve-or-tools-cpsat PRIVATE main-cpsat.cpp cj/cj-csp.c cj/cj-csp-io.c)
target_link_libraries(cj-solve-or-tools-cpsat PUBLIC ortools)
target_include_directories(cj-solve-or-tools-cpsat AFTER PUBLIC ../)
install(TARGETS cj-solve-or-tools-cpsat DESTINATION .)
