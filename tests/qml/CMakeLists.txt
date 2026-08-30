# Qt Quick Test harness. Scans this directory for tst_*.qml files at run
# time. Links the backing library + its plugin so ModernHAMLoggerQt QML
# types are importable from test QML the same way the app imports them.
qt_add_executable(tst_qmlharness tst_qmlharness.cpp)
target_compile_definitions(tst_qmlharness PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(tst_qmlharness PRIVATE
    hamloggercore
    hamloggercoreplugin
    Qt6::QuickTest
    Qt6::Qml
)

# -platform offscreen: a Quick Test always opens a window otherwise, even
# for tests that touch no UI.
add_test(NAME tst_qmlharness COMMAND tst_qmlharness -platform offscreen)
set_tests_properties(tst_qmlharness PROPERTIES
    ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
)
