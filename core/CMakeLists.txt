add_library(wm_core STATIC
    include/core/word_selection.h
    src/word_selection.cpp
    include/core/practice_session.h
    src/practice_session.cpp
)

target_include_directories(wm_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(wm_core PUBLIC
    wm_db
    Qt${QT_VERSION_MAJOR}::Core
)