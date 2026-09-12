add_executable(tst_core
    test_word_selection.cpp
)

target_link_libraries(tst_core PRIVATE wm_core)

add_test(NAME word_selection COMMAND tst_core)