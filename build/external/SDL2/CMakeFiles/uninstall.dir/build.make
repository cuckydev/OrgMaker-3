# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/cmake/340/bin/cmake

# The command to remove a file.
RM = /snap/cmake/340/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cuckydev/OrgMaker3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cuckydev/OrgMaker3/build

# Utility rule file for uninstall.

# Include the progress variables for this target.
include external/SDL2/CMakeFiles/uninstall.dir/progress.make

external/SDL2/CMakeFiles/uninstall:
	cd /home/cuckydev/OrgMaker3/build/external/SDL2 && /snap/cmake/340/bin/cmake -P /home/cuckydev/OrgMaker3/build/external/SDL2/cmake_uninstall.cmake

uninstall: external/SDL2/CMakeFiles/uninstall
uninstall: external/SDL2/CMakeFiles/uninstall.dir/build.make

.PHONY : uninstall

# Rule to build all files generated by this target.
external/SDL2/CMakeFiles/uninstall.dir/build: uninstall

.PHONY : external/SDL2/CMakeFiles/uninstall.dir/build

external/SDL2/CMakeFiles/uninstall.dir/clean:
	cd /home/cuckydev/OrgMaker3/build/external/SDL2 && $(CMAKE_COMMAND) -P CMakeFiles/uninstall.dir/cmake_clean.cmake
.PHONY : external/SDL2/CMakeFiles/uninstall.dir/clean

external/SDL2/CMakeFiles/uninstall.dir/depend:
	cd /home/cuckydev/OrgMaker3/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cuckydev/OrgMaker3 /home/cuckydev/OrgMaker3/external/SDL2 /home/cuckydev/OrgMaker3/build /home/cuckydev/OrgMaker3/build/external/SDL2 /home/cuckydev/OrgMaker3/build/external/SDL2/CMakeFiles/uninstall.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : external/SDL2/CMakeFiles/uninstall.dir/depend

