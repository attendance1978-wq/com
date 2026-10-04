cmake_minimum_required(VERSION 3.10)
project(com VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Everything except main.c goes into a library so tools and tests can link it.
file(GLOB_RECURSE COM_LIB_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.c)
list(FILTER COM_LIB_SOURCES EXCLUDE REGEX ".*/src/main\\.c$")

add_library(comlib STATIC ${COM_LIB_SOURCES})
target_include_directories(comlib PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
if(NOT MSVC)
  target_compile_options(comlib PRIVATE -Wall -Wextra)
endif()

add_executable(com src/main.c)
target_link_libraries(com PRIVATE comlib)

foreach(tool comfmt comlint comrepl)
  add_executable(${tool} tools/${tool}/${tool}.c)
  target_link_libraries(${tool} PRIVATE comlib)
endforeach()

foreach(b bench_lexer bench_parser)
  add_executable(${b} EXCLUDE_FROM_ALL bench/${b}.c)
  target_link_libraries(${b} PRIVATE comlib)
endforeach()

# ---- tests (ctest) ----
enable_testing()
foreach(t unit/test_lexer unit/test_parser unit/test_sema unit/test_ir
          integration/test_compile integration/test_vm)
  get_filename_component(name ${t} NAME)
  add_executable(${name} tests/${t}.c)
  target_link_libraries(${name} PRIVATE comlib)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
  add_test(NAME ${name} COMMAND ${name})
endforeach()

add_test(NAME programs
         COMMAND ${CMAKE_COMMAND} -E env COM=$<TARGET_FILE:com> sh tests/run_tests.sh
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ---- install ----
install(TARGETS com comfmt comlint comrepl RUNTIME DESTINATION bin)
install(DIRECTORY stdlib/ DESTINATION share/com/stdlib FILES_MATCHING PATTERN "*.com")
