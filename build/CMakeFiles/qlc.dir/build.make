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

# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/qlc/webserver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/qlc/webserver/build

# Include any dependencies generated for this target.
include CMakeFiles/qlc.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/qlc.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/qlc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/qlc.dir/flags.make

CMakeFiles/qlc.dir/src/qlc_log.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/qlc_log.cpp.o: ../src/qlc_log.cpp
CMakeFiles/qlc.dir/src/qlc_log.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/qlc.dir/src/qlc_log.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/qlc_log.cpp.o -MF CMakeFiles/qlc.dir/src/qlc_log.cpp.o.d -o CMakeFiles/qlc.dir/src/qlc_log.cpp.o -c /home/qlc/webserver/src/qlc_log.cpp

CMakeFiles/qlc.dir/src/qlc_log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/qlc_log.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/qlc_log.cpp > CMakeFiles/qlc.dir/src/qlc_log.cpp.i

CMakeFiles/qlc.dir/src/qlc_log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/qlc_log.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/qlc_log.cpp -o CMakeFiles/qlc.dir/src/qlc_log.cpp.s

CMakeFiles/qlc.dir/src/util.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/util.cpp.o: ../src/util.cpp
CMakeFiles/qlc.dir/src/util.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/qlc.dir/src/util.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/util.cpp.o -MF CMakeFiles/qlc.dir/src/util.cpp.o.d -o CMakeFiles/qlc.dir/src/util.cpp.o -c /home/qlc/webserver/src/util.cpp

CMakeFiles/qlc.dir/src/util.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/util.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/util.cpp > CMakeFiles/qlc.dir/src/util.cpp.i

CMakeFiles/qlc.dir/src/util.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/util.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/util.cpp -o CMakeFiles/qlc.dir/src/util.cpp.s

CMakeFiles/qlc.dir/src/config.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/config.cpp.o: ../src/config.cpp
CMakeFiles/qlc.dir/src/config.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/qlc.dir/src/config.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/config.cpp.o -MF CMakeFiles/qlc.dir/src/config.cpp.o.d -o CMakeFiles/qlc.dir/src/config.cpp.o -c /home/qlc/webserver/src/config.cpp

CMakeFiles/qlc.dir/src/config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/config.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/config.cpp > CMakeFiles/qlc.dir/src/config.cpp.i

CMakeFiles/qlc.dir/src/config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/config.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/config.cpp -o CMakeFiles/qlc.dir/src/config.cpp.s

CMakeFiles/qlc.dir/src/thread.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/thread.cpp.o: ../src/thread.cpp
CMakeFiles/qlc.dir/src/thread.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/qlc.dir/src/thread.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/thread.cpp.o -MF CMakeFiles/qlc.dir/src/thread.cpp.o.d -o CMakeFiles/qlc.dir/src/thread.cpp.o -c /home/qlc/webserver/src/thread.cpp

CMakeFiles/qlc.dir/src/thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/thread.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/thread.cpp > CMakeFiles/qlc.dir/src/thread.cpp.i

CMakeFiles/qlc.dir/src/thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/thread.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/thread.cpp -o CMakeFiles/qlc.dir/src/thread.cpp.s

CMakeFiles/qlc.dir/src/mutex.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/mutex.cpp.o: ../src/mutex.cpp
CMakeFiles/qlc.dir/src/mutex.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/qlc.dir/src/mutex.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/mutex.cpp.o -MF CMakeFiles/qlc.dir/src/mutex.cpp.o.d -o CMakeFiles/qlc.dir/src/mutex.cpp.o -c /home/qlc/webserver/src/mutex.cpp

CMakeFiles/qlc.dir/src/mutex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/mutex.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/mutex.cpp > CMakeFiles/qlc.dir/src/mutex.cpp.i

CMakeFiles/qlc.dir/src/mutex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/mutex.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/mutex.cpp -o CMakeFiles/qlc.dir/src/mutex.cpp.s

CMakeFiles/qlc.dir/src/fiber.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/fiber.cpp.o: ../src/fiber.cpp
CMakeFiles/qlc.dir/src/fiber.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/qlc.dir/src/fiber.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/fiber.cpp.o -MF CMakeFiles/qlc.dir/src/fiber.cpp.o.d -o CMakeFiles/qlc.dir/src/fiber.cpp.o -c /home/qlc/webserver/src/fiber.cpp

CMakeFiles/qlc.dir/src/fiber.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/fiber.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/fiber.cpp > CMakeFiles/qlc.dir/src/fiber.cpp.i

CMakeFiles/qlc.dir/src/fiber.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/fiber.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/fiber.cpp -o CMakeFiles/qlc.dir/src/fiber.cpp.s

CMakeFiles/qlc.dir/src/schedular.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/schedular.cpp.o: ../src/schedular.cpp
CMakeFiles/qlc.dir/src/schedular.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/qlc.dir/src/schedular.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/schedular.cpp.o -MF CMakeFiles/qlc.dir/src/schedular.cpp.o.d -o CMakeFiles/qlc.dir/src/schedular.cpp.o -c /home/qlc/webserver/src/schedular.cpp

CMakeFiles/qlc.dir/src/schedular.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/schedular.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/schedular.cpp > CMakeFiles/qlc.dir/src/schedular.cpp.i

CMakeFiles/qlc.dir/src/schedular.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/schedular.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/schedular.cpp -o CMakeFiles/qlc.dir/src/schedular.cpp.s

CMakeFiles/qlc.dir/src/iomanager.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/iomanager.cpp.o: ../src/iomanager.cpp
CMakeFiles/qlc.dir/src/iomanager.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/qlc.dir/src/iomanager.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/iomanager.cpp.o -MF CMakeFiles/qlc.dir/src/iomanager.cpp.o.d -o CMakeFiles/qlc.dir/src/iomanager.cpp.o -c /home/qlc/webserver/src/iomanager.cpp

CMakeFiles/qlc.dir/src/iomanager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/iomanager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/iomanager.cpp > CMakeFiles/qlc.dir/src/iomanager.cpp.i

CMakeFiles/qlc.dir/src/iomanager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/iomanager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/iomanager.cpp -o CMakeFiles/qlc.dir/src/iomanager.cpp.s

CMakeFiles/qlc.dir/src/timer.cpp.o: CMakeFiles/qlc.dir/flags.make
CMakeFiles/qlc.dir/src/timer.cpp.o: ../src/timer.cpp
CMakeFiles/qlc.dir/src/timer.cpp.o: CMakeFiles/qlc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/qlc.dir/src/timer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/qlc.dir/src/timer.cpp.o -MF CMakeFiles/qlc.dir/src/timer.cpp.o.d -o CMakeFiles/qlc.dir/src/timer.cpp.o -c /home/qlc/webserver/src/timer.cpp

CMakeFiles/qlc.dir/src/timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/qlc.dir/src/timer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qlc/webserver/src/timer.cpp > CMakeFiles/qlc.dir/src/timer.cpp.i

CMakeFiles/qlc.dir/src/timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/qlc.dir/src/timer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qlc/webserver/src/timer.cpp -o CMakeFiles/qlc.dir/src/timer.cpp.s

# Object files for target qlc
qlc_OBJECTS = \
"CMakeFiles/qlc.dir/src/qlc_log.cpp.o" \
"CMakeFiles/qlc.dir/src/util.cpp.o" \
"CMakeFiles/qlc.dir/src/config.cpp.o" \
"CMakeFiles/qlc.dir/src/thread.cpp.o" \
"CMakeFiles/qlc.dir/src/mutex.cpp.o" \
"CMakeFiles/qlc.dir/src/fiber.cpp.o" \
"CMakeFiles/qlc.dir/src/schedular.cpp.o" \
"CMakeFiles/qlc.dir/src/iomanager.cpp.o" \
"CMakeFiles/qlc.dir/src/timer.cpp.o"

# External object files for target qlc
qlc_EXTERNAL_OBJECTS =

../lib/libqlc.so: CMakeFiles/qlc.dir/src/qlc_log.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/util.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/config.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/thread.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/mutex.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/fiber.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/schedular.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/iomanager.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/src/timer.cpp.o
../lib/libqlc.so: CMakeFiles/qlc.dir/build.make
../lib/libqlc.so: CMakeFiles/qlc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/qlc/webserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking CXX shared library ../lib/libqlc.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/qlc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/qlc.dir/build: ../lib/libqlc.so
.PHONY : CMakeFiles/qlc.dir/build

CMakeFiles/qlc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/qlc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/qlc.dir/clean

CMakeFiles/qlc.dir/depend:
	cd /home/qlc/webserver/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/qlc/webserver /home/qlc/webserver /home/qlc/webserver/build /home/qlc/webserver/build /home/qlc/webserver/build/CMakeFiles/qlc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/qlc.dir/depend

