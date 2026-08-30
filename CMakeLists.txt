cmake_minimum_required(VERSION 3.16)

project(ModernHAMLoggerQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appModernHAMLoggerQt
    main.cpp
)

# pragma Singleton QML files must have this source property set before the
# qt_add_qml_module() call, otherwise the module's qmldir won't mark them
# as singletons.
set_source_files_properties(qml/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(appModernHAMLoggerQt
    URI ModernHAMLoggerQt
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

target_include_directories(appModernHAMLoggerQt
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

target_link_libraries(appModernHAMLoggerQt
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Network
)

install(TARGETS appModernHAMLoggerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
