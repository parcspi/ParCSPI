
if(DEFINED IN_SOURCE_BUILD)
    set(LLVM_LINK_COMPONENTS BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support Svf)
    add_llvm_tool( dvf dda.cpp )
else()
    add_executable( dvf dda.cpp )

    target_link_libraries( dvf Svf ${llvm_libs} )

    set_target_properties( dvf PROPERTIES
                           RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
endif()
