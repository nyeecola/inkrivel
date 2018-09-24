#include "../../../lib/catch.hpp"
#include "character.hpp"

TEST_CASE( "Character has name" ) {
    Character c("abc", Weapon("name", 0, 0));
    REQUIRE( c.name == "abc" );
}

TEST_CASE( "Character has not generic name" ) {
    Character c("", Weapon("name", 0, 0));
    REQUIRE( c.name != "abc" );
}
