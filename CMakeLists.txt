# Set project name
project (LD37)
# Define target name
set (TARGET_NAME ld37)

################################

# Set CMake minimum version and CMake policy required by Urho3D-Cmake-Common module
if (WIN32)
	cmake_minimum_required (VERSION 3.2.3) # Going forward all platforms will use this
else()
	cmake_minimum_required (VERSION 2.8.6)
endif()
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
	if (CMAKE_VERSION VERSION_GREATER 2.8.12 OR CMAKE_VERSION VERSION_EQUAL 2.8.12)
		# INTERFACE_LINK LIBRARIES defines the link interface
		cmake_policy (SET CMP0022 NEW)
	endif()
	if (CMAKE_VERSION VERSION_GREATER 3.0.0 OR CMAKE_VERSION VERSION_EQUAL 3.0.0)
		# Disallow use of the LOCATION target property - so we set OLD as we still need it
		cmake_policy (SET CMP0026 OLD)
		# MACOSX_RPATH is enabled by default
		cmake_policy (SET CMP0042 NEW)
	endif()
endif()
# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Cmake/Modules)
# Include Urho3D CMake common module
include (Urho3D-CMake-Common)
# Enable C++11, because inline functions
set (CMAKE_CXX_FLAGS "--std=c++11")
# Include headers
include_directories(Sources)
# Define source files
define_source_files(RECURSE true GLOB_CPP_PATTERNS Sources/*.cpp)
# Setup target with resource copying
setup_main_executable()
