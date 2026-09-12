add_library(wm_db STATIC
    include/data/models.h
    include/data/word_database.h
    src/word_database.cpp
)

target_include_directories(wm_db PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(wm_db PUBLIC Qt${QT_VERSION_MAJOR}::Core)