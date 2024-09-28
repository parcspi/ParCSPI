set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message("@@@@@@@@@${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})


add_library(pac_analysis SHARED pac_analysis.cpp pac_analysis.h dependency_walker.cpp instrumenter_loger.cpp csdda.cpp)
target_link_libraries(pac_analysis PUBLIC Svf) # ${llvm_libs} Threads::Threads) 
                                #  Svf)
                                #  ${llvm_bitwriter}
                                #  ${llvm_irreader})

link_libraries(pac_analysis)

set_target_properties(pac_analysis PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(pac_analysis PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(pac_analysis PROPERTIES
	COMPILE_FLAGS "-fno-rtti"
)


