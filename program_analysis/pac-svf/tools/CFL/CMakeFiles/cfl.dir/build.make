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

# Include any dependencies generated for this target.
include tools/CFL/CMakeFiles/cfl.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/CFL/CMakeFiles/cfl.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/CFL/CMakeFiles/cfl.dir/progress.make

# Include the compile flags for this target's objects.
include tools/CFL/CMakeFiles/cfl.dir/flags.make

tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o: tools/CFL/CMakeFiles/cfl.dir/flags.make
tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o: tools/CFL/cfl.cpp
tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o: tools/CFL/CMakeFiles/cfl.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ub1804/pgi_ws/SVF/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o"
	cd /home/ub1804/pgi_ws/SVF/tools/CFL && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o -MF CMakeFiles/cfl.dir/cfl.cpp.o.d -o CMakeFiles/cfl.dir/cfl.cpp.o -c /home/ub1804/pgi_ws/SVF/tools/CFL/cfl.cpp

tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cfl.dir/cfl.cpp.i"
	cd /home/ub1804/pgi_ws/SVF/tools/CFL && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ub1804/pgi_ws/SVF/tools/CFL/cfl.cpp > CMakeFiles/cfl.dir/cfl.cpp.i

tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cfl.dir/cfl.cpp.s"
	cd /home/ub1804/pgi_ws/SVF/tools/CFL && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ub1804/pgi_ws/SVF/tools/CFL/cfl.cpp -o CMakeFiles/cfl.dir/cfl.cpp.s

# Object files for target cfl
cfl_OBJECTS = \
"CMakeFiles/cfl.dir/cfl.cpp.o"

# External object files for target cfl
cfl_EXTERNAL_OBJECTS =

bin/cfl: tools/CFL/CMakeFiles/cfl.dir/cfl.cpp.o
bin/cfl: tools/CFL/CMakeFiles/cfl.dir/build.make
bin/cfl: lib/libSvf.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMBitWriter.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMCore.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMipo.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMIRReader.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMInstCombine.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMInstrumentation.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMTarget.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMLinker.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMAnalysis.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMScalarOpts.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMSupport.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMTransformUtils.a
bin/cfl: z3.obj/bin/libz3.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMBitWriter.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMAsmParser.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMInstCombine.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMAggressiveInstCombine.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMFrontendOpenMP.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMVectorize.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMTransformUtils.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMAnalysis.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMObject.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMBitReader.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMMCParser.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMTextAPI.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMProfileData.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMCore.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMRemarks.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMBitstreamReader.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMMC.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMBinaryFormat.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMDebugInfoCodeView.a
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMSupport.a
bin/cfl: /usr/lib/x86_64-linux-gnu/libz.so
bin/cfl: /usr/lib/x86_64-linux-gnu/libtinfo.so
bin/cfl: /home/ub1804/jay_ws/llvm-project/build/lib/libLLVMDemangle.a
bin/cfl: tools/CFL/CMakeFiles/cfl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ub1804/pgi_ws/SVF/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/cfl"
	cd /home/ub1804/pgi_ws/SVF/tools/CFL && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cfl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/CFL/CMakeFiles/cfl.dir/build: bin/cfl
.PHONY : tools/CFL/CMakeFiles/cfl.dir/build

tools/CFL/CMakeFiles/cfl.dir/clean:
	cd /home/ub1804/pgi_ws/SVF/tools/CFL && $(CMAKE_COMMAND) -P CMakeFiles/cfl.dir/cmake_clean.cmake
.PHONY : tools/CFL/CMakeFiles/cfl.dir/clean

tools/CFL/CMakeFiles/cfl.dir/depend:
	cd /home/ub1804/pgi_ws/SVF && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ub1804/pgi_ws/SVF /home/ub1804/pgi_ws/SVF/tools/CFL /home/ub1804/pgi_ws/SVF /home/ub1804/pgi_ws/SVF/tools/CFL /home/ub1804/pgi_ws/SVF/tools/CFL/CMakeFiles/cfl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/CFL/CMakeFiles/cfl.dir/depend

