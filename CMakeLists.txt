cmake_minimum_required(VERSION 3.22)

project(ModernHAMLoggerQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Quick Network Test QuickTest)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

# Backend + QML sources live in a static library so both the app and the
# test binaries can link them without recompiling. STATIC (rather than the
# SHARED default for this Qt build) keeps the app a single .exe with no
# extra DLL to deploy.
qt_add_library(hamloggercore STATIC)

# pragma Singleton QML files must have this source property set before the
# qt_add_qml_module() call, otherwise the module's qmldir won't mark them
# as singletons.
set_source_files_properties(qml/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(hamloggercore
    URI ModernHAMLoggerQt
    VERSION 1.0
    QML_FILES
        Main.qml
        qml/Theme.qml
        qml/CheckIndicator.qml
        qml/OperationSidebar.qml
        qml/NewOperationDialog.qml
        qml/StationProfileDialog.qml
        qml/EntryForm.qml
        qml/LogTable.qml
    SOURCES
        backend/qso.h
        backend/qsologmodel.h
        backend/qsologmodel.cpp
        backend/operation.h
        backend/operation.cpp
        backend/operationlistmodel.h
        backend/operationlistmodel.cpp
        backend/stationprofile.h
        backend/stationprofile.cpp
        backend/adifio.h
        backend/adifio.cpp
        backend/callsignlookupservice.h
        backend/callsignlookupservice.cpp
        backend/logbookmanager.h
        backend/logbookmanager.cpp
)

# PUBLIC: backend headers are included flat (e.g. "qso.h") by sibling
# backend files, and test targets linking hamloggercore need the same path.
target_include_directories(hamloggercore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/backend)

target_link_libraries(hamloggercore
    PUBLIC Qt6::Quick
    PUBLIC Qt6::Network
)

qt_add_executable(appModernHAMLoggerQt
    main.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appModernHAMLoggerQt PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appModernHAMLoggerQt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Linking a STATIC QML module's backing library isn't enough on its own --
# the module's generated plugin target must be linked explicitly too, and
# main.cpp imports it statically via Q_IMPORT_QML_PLUGIN.
target_link_libraries(appModernHAMLoggerQt
    PRIVATE hamloggercore
    PRIVATE hamloggercoreplugin
)

install(TARGETS appModernHAMLoggerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(MHL_BUILD_TESTS "Build the test suite" ON)
if(MHL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
