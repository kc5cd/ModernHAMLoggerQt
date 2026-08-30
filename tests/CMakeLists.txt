# Links a Qt6::Test executable against the backend library, registers it
# with ctest, and makes sure the Qt runtime DLLs are on PATH at run time --
# without this, a test binary on Windows/MinGW exits silently and ctest
# reports a bare failure with no diagnostic.
function(mhl_add_cpp_test name)
    qt_add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE hamloggercore Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endfunction()

add_subdirectory(cpp)
add_subdirectory(qml)
