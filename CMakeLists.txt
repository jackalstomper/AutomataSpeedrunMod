cmake_minimum_required(VERSION 3.10)
project(AutomataSpeedrunMod)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DWIN32_LEAN_AND_MEAN -DAUTOMATA_LOG)
add_definitions(-D__WRL_CLASSIC_COM_STRICT__)

if(CMAKE_BUILD_TYPE EQUAL Debug)
	add_definitions(-D_ATL_DEBUG_INTERFACES)
endif()

if(CMAKE_BUILD_TYPE EQUAL Release)
	add_definitions(-DAUTOMATA_RELEASE_TARGET)
endif()

include_directories(src)

add_subdirectory(fmt)

file(GLOB_RECURSE MOD_SOURCES src/*.cpp)
add_library(xinput1_4 SHARED DLLMain.cpp ${MOD_SOURCES} xinput.def)
target_link_libraries(xinput1_4 fmt::fmt d3d11 d2d1 dwrite dxgi bcrypt)
