find_package(Qt6 REQUIRED COMPONENTS Core Xml Network Test)

add_executable(test_dlnaparser
    test_dlnaparser.h
    test_dlnaparser.cpp
    ../src/dlnaparser.cpp
)
target_include_directories(test_dlnaparser PRIVATE ../src)
target_link_libraries(test_dlnaparser PRIVATE
    Qt6::Core
    Qt6::Xml
    Qt6::Network
    Qt6::Test
)
add_test(NAME test_dlnaparser COMMAND test_dlnaparser)
