cmake_minimum_required(VERSION 3.8)

#########
# Setup #
#########

# Define project, source, and includes
project(OrgMaker3 LANGUAGES CXX)

add_executable(OrgMaker3 WIN32
	"src/Main.cpp"
	"src/Error.cpp"
	"src/Error.h"
	"src/FixedPoint.h"
	"src/ContentProvider.cpp"
	"src/ContentProvider.h"
	"src/Audio.cpp"
	"src/Audio.h"
	"src/Window.h"
	"src/Placement.h"
	"src/Task.h"
	"src/Task_EditorWindow.cpp"
	"src/Task_EditorWindow.h"
	"src/Organya.cpp"
	"src/Organya.h"
)

target_include_directories(OrgMaker3 PRIVATE "src")

set_target_properties(OrgMaker3 PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

#######################
# CGE Backend Linking #
#######################
find_package(PkgConfig QUIET)

# Link SDL2
find_package(SDL2)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(sdl2 QUIET IMPORTED_TARGET sdl2)
endif()
target_compile_options(OrgMaker3 PRIVATE ${sdl2_CFLAGS})
target_link_libraries(OrgMaker3 PRIVATE ${sdl2_LDFLAGS})

##########
# Tweaks #
##########
# This is messy as hell, and has been replaced by CMAKE_MSVC_RUNTIME_LIBRARY,
# but that's a very recent CMake addition, so we're still doing it this way for now
if(MSVC AND MSVC_LINK_STATIC_RUNTIME)
	# Statically-link the CRT (vcpkg static libs do this)
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

# Do some other MSVC fixes
if(MSVC)
	# Disable warnings that normally fire up on MSVC when using "unsafe" functions instead of using MSVC's "safe" _s functions
	target_compile_definitions(OrgMaker3 PRIVATE _CRT_SECURE_NO_WARNINGS)
	
	# Make it so source files are recognized as UTF-8 by MSVC
	target_compile_options(OrgMaker3 PRIVATE "/utf-8")
	
	# Use `main` instead of `WinMain`
	set_target_properties(OrgMaker3 PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()
