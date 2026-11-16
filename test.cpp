#include <catch2/catch_test_macros.hpp>

int add(int a , int b){
return a + b ;
}
TEST_CASE("Addition test"){
REQUIRE(add(2,3) == 5);
REQUIRE(add(5,5) ==10);
CHECK(add(-1,1) ==0);
}
