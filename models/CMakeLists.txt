add_library(wm_models STATIC
    include/models/word_list_model.h
    src/word_list_model.cpp
    include/models/practice_record_model.h
    src/practice_record_model.cpp
)

target_include_directories(wm_models PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(wm_models PUBLIC
    wm_db
    Qt${QT_VERSION_MAJOR}::Core
)