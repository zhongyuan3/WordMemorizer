set(APP_SOURCES
    main.cpp
    main_window.h
    main_window.cpp
    ${PROJECT_SOURCE_DIR}/resources/resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WordMemorizer
        MANUAL_FINALIZATION
        ${APP_SOURCES}
    )
else()
    if(ANDROID)
        add_library(WordMemorizer SHARED
            ${APP_SOURCES}
        )
    else()
        add_executable(WordMemorizer
            ${APP_SOURCES}
        )
    endif()
endif()

target_include_directories(WordMemorizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(WordMemorizer PRIVATE
    wm_core
    wm_db
    wm_models
    wm_widgets
    Qt${QT_VERSION_MAJOR}::Widgets
)

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