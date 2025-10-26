cmake_minimum_required(VERSION 3.20)
project(moko3)

include(cmake/deps.cmake)

option(moko3_ENABLE_TESTING "enables testing" OFF)
option(moko3_ENABLE_ASAN "build ASAN (for CI)" OFF)
option(moko3_ENABLE_UBSAN "build UNSAN (for CI)" OFF)

add_library(moko3lib STATIC
    # add new .cpp files here. GLOB for libraries is bad practice  
    src/common.cpp
    src/sections.cpp
    src/testbox.cpp
)

set_target_properties(moko3lib PROPERTIES
		LINKER_LANGUAGE CXX
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON)
target_include_directories(moko3lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(moko3lib PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    anyanylib
    kelcorolib
    zallib
    clinoklib
)

if(moko3_ENABLE_TESTING)
	include(CTest)
	add_subdirectory(test)
endif()

# generates launch.json, places compile_commands.json correctly for clangd
ccozy_on_configuration_end_vscode()
