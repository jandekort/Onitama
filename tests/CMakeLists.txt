# GoogleTest is fetched at configure time (needs network on first configure;
# afterwards it is cached under the build directory).
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
# Don't let GoogleTest override the parent project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(onitama_tests
        engine_test.cpp)
target_link_libraries(onitama_tests PRIVATE onitama_engine GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(onitama_tests)
