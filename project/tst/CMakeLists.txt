add_executable(first_test
  tst/main_test.cpp
)

target_link_libraries(first_test
  PRIVATE
  ${GTEST_LIBRARIES}
)

enable_testing()
add_test(NAME first_test COMMAND first_test)