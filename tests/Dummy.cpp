#include <catch2/catch_test_macros.hpp>

// this will allow our pipelines not to fail due to zero test cases found
TEST_CASE ("Placeholder", "[placeholder]")
{
    REQUIRE (true);
}
