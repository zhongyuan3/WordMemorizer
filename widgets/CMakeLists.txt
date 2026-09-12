add_library(wm_widgets STATIC
    include/widgets/definition_editor_widget.h
    src/definition_editor_widget.cpp
    include/widgets/edit_word_dialog.h
    src/edit_word_dialog.cpp
    include/widgets/practice_record_detail.h
    src/practice_record_detail.cpp
    include/widgets/practice_record_widget.h
    src/practice_record_widget.cpp
    include/widgets/practice_result_delegate.h
    src/practice_result_delegate.cpp
    include/widgets/practice_widget.h
    src/practice_widget.cpp
    include/widgets/word_input_widget.h
    src/word_input_widget.cpp
    include/widgets/word_list_delegate.h
    src/word_list_delegate.cpp
    include/widgets/word_list_widget.h
    src/word_list_widget.cpp
)

target_include_directories(wm_widgets PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(wm_widgets PUBLIC
    wm_db                      # practice_record_detail.h 等头文件包含 data/models.h
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_link_libraries(wm_widgets PRIVATE
    wm_core                    # practice_widget.cpp 使用 PracticeSession
    wm_models                  # word_list_widget/delegate 使用 WordListModel
)