set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_program(CCACHE_PROGRAM sccache)

if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "Using sccache")
endif()

find_program(MOLD_LINKER mold)
find_program(GOLD_LINKER gold)

if(MOLD_LINKER)
  add_link_options("-fuse-ld=mold")
  message(STATUS "Using mold as linker")
elseif(GOLD_LINKER)
  add_link_options("-fuse-ld=gold")
  message(STATUS "Using gold as linker")
else()
  message(STATUS "Default linker will be used")
endif()

add_compile_options(
  -Wall
  -Wextra
  -pedantic
  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Woverloaded-virtual
  -Wdouble-promotion
  -Wformat=2
  -Wconversion
  -Wsign-conversion
  -Wimplicit-fallthrough)
add_compile_options(-ftemplate-backtrace-limit=2 -fdiagnostics-color=always -fdiagnostics-show-template-tree)
add_compile_options(-fPIE)
add_link_options(-pie)

message(STATUS "Compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-ftrivial-auto-var-init=pattern -fno-rtti)
  add_compile_options(-Og -g3 -gsplit-dwarf)
  add_compile_options(-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -fasynchronous-unwind-tables)
  add_compile_options(-fsanitize=address,undefined,leak)
  add_link_options(-fsanitize=address,undefined,leak)
  add_link_options(-static-libstdc++ -static-libgcc)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG)
  else()
    add_compile_options(-Wtrampolines)
    add_definitions(-D_GLIBCXX_DEBUG)
    add_compile_options(-fmax-errors=2)
  endif()
else()
  add_compile_options(-ftrivial-auto-var-init=zero -fno-rtti)
  add_compile_options(-D_FORTIFY_SOURCE=3)
  add_compile_options(-O3 -DNDEBUG -flto=4)
  add_link_options(-flto=4)
  add_link_options(-static-libstdc++ -static-libgcc)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE)
  else()
    add_compile_options(-Wtrampolines)
    add_definitions(-D_GLIBCXX_ASSERTIONS)
    add_compile_options(-fmax-errors=2)
  endif()
endif()
