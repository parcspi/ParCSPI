# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ub1804/pgi_ws/SVF

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ub1804/pgi_ws/SVF

# Utility rule file for install-Svf.

# Include any custom commands dependencies for this target.
include lib/CMakeFiles/install-Svf.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/CMakeFiles/install-Svf.dir/progress.make

lib/CMakeFiles/install-Svf:
	cd /home/ub1804/pgi_ws/SVF/lib && /usr/local/bin/cmake -DCMAKE_INSTALL_COMPONENT="Svf" -P /home/ub1804/pgi_ws/SVF/cmake_install.cmake

install-Svf: lib/CMakeFiles/install-Svf
install-Svf: lib/CMakeFiles/install-Svf.dir/build.make
.PHONY : install-Svf

# Rule to build all files generated by this target.
lib/CMakeFiles/install-Svf.dir/build: install-Svf
.PHONY : lib/CMakeFiles/install-Svf.dir/build

lib/CMakeFiles/install-Svf.dir/clean:
	cd /home/ub1804/pgi_ws/SVF/lib && $(CMAKE_COMMAND) -P CMakeFiles/install-Svf.dir/cmake_clean.cmake
.PHONY : lib/CMakeFiles/install-Svf.dir/clean

lib/CMakeFiles/install-Svf.dir/depend:
	cd /home/ub1804/pgi_ws/SVF && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ub1804/pgi_ws/SVF /home/ub1804/pgi_ws/SVF/lib /home/ub1804/pgi_ws/SVF /home/ub1804/pgi_ws/SVF/lib /home/ub1804/pgi_ws/SVF/lib/CMakeFiles/install-Svf.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/CMakeFiles/install-Svf.dir/depend

