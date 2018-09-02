#include "catch.hpp"
#include "weapon.hpp"

TEST_CASE( "Weapon can't have negative damage" ) {
    Weapon w("abc", -2, 10);
    REQUIRE( w.getDamage() == 1 );
}
