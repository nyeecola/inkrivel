#include "catch.hpp"
#include "character.hpp"

TEST_CASE( "Character has name" ) {
    Character c("abc");
    REQUIRE( c.getName() == "abc" );
}

TEST_CASE( "Character has not generic name" ) {
    Character c("");
    REQUIRE( c.getName() != "abc" );
}
