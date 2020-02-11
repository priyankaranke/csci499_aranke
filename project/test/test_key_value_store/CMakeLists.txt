file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(test_key_value_store ${SRCS})

TARGET_LINK_LIBRARIES(test_key_value_store
    Warble
    libgtest
)

add_test(NAME test_key_value_store
         COMMAND test_key_value_store)