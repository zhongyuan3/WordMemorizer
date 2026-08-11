cmake_minimum_required(VERSION 3.16)

project(WordMemorizer VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp

        src/main_window.h
        src/main_window.cpp

        src/data/models.h
        src/data/word_database.h
        src/data/word_database.cpp

        src/core/word_selection.h
        src/core/word_selection.cpp
        src/core/practice_session.h
        src/core/practice_session.cpp

        src/models/word_list_model.h
        src/models/word_list_model.cpp
        src/models/practice_record_model.h
        src/models/practice_record_model.cpp

        src/widgets/definition_editor_widget.h
        src/widgets/definition_editor_widget.cpp
        src/widgets/practice_result_delegate.h
        src/widgets/practice_result_delegate.cpp
        src/widgets/word_input_widget.h
        src/widgets/word_input_widget.cpp
        src/widgets/word_list_widget.h
        src/widgets/word_list_widget.cpp
        src/widgets/word_list_delegate.h
        src/widgets/word_list_delegate.cpp
        src/widgets/edit_word_dialog.h
        src/widgets/edit_word_dialog.cpp
        src/widgets/practice_widget.h
        src/widgets/practice_widget.cpp
        src/widgets/practice_record_widget.h
        src/widgets/practice_record_widget.cpp
        src/widgets/practice_record_detail.h
        src/widgets/practice_record_detail.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WordMemorizer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    qt6_add_resources(WordMemorizer "resources"
        PREFIX "/"
        FILES
            styles/main.qss
            icons/up_arrow_normal.png
            icons/up_arrow_pressed.png
            icons/up_arrow_hover.png
            icons/down_arrow_normal.png
            icons/down_arrow_pressed.png
            icons/down_arrow_hover.png
            icons/dropdown.png
            icons/logo.png
    )
else()
    if(ANDROID)
        add_library(WordMemorizer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(WordMemorizer
            ${PROJECT_SOURCES}
        )
    endif()

    target_sources(WordMemorizer PRIVATE resources.qrc)
endif()

target_include_directories(WordMemorizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(WordMemorizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WordMemorizer)
endif()
set_target_properties(WordMemorizer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WordMemorizer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WordMemorizer)
endif()
