#include "GravityDataProduct.h"
#include "../doctest.h"

#include <string>
#include <iostream>

using namespace gravity;

TEST_CASE("Tests without the mocking framework")
{
    //only functions that are not setters/getters
    SUBCASE("Test equality of GravityDataProdcuts")
    {
        GravityDataProduct gdp("testProductID");
        char data[12];
        sprintf(data, "Hello World");
        gdp.setData((void*)data, strlen(data));

        GIVEN("two DataProducts with the same dataProductIDs and same data.")
        {
            GravityDataProduct gdp2("testProductID");
            char data2[12];
            sprintf(data2, "Hello World");
            gdp2.setData((void*)data2, strlen(data2));
            THEN("they should be equal")
            {
                CHECK(gdp == gdp2);
                CHECK(gdp2 == gdp);
                CHECK(!(gdp != gdp2));
                CHECK(!(gdp2 != gdp));
            }
        }

        GIVEN("two DataProducts with different dataProductIDs and the same data.")
        {
            GravityDataProduct gdp2("testProductID_2");
            char data2[12];
            sprintf(data2, "Hello World");
            gdp2.setData((void*)data2, strlen(data2));
            THEN("they should not be equal")
            {
                CHECK(gdp != gdp2);
                CHECK(gdp2 != gdp);
                CHECK(!(gdp == gdp2));
                CHECK(!(gdp2 == gdp));
            }
        }

        GIVEN("two DataProducts with the same dataProductIDs and different data.")
        {
            GravityDataProduct gdp2("testProductID");
            char data2[12];
            sprintf(data2, "hello world");
            gdp2.setData((void*)data2, strlen(data2));
            THEN("they should not be equal")
            {
                CHECK(gdp != gdp2);
                CHECK(gdp2 != gdp);
                CHECK(!(gdp == gdp2));
                CHECK(!(gdp2 == gdp));
            }
        }

        GIVEN("two DataProducts with different dataProductIDs and different data.")
        {
            GravityDataProduct gdp2("testProductID_");
            char data2[12];
            sprintf(data2, "hello world");
            gdp2.setData((void*)data2, strlen(data2));
            THEN("they should not be equal")
            {
                CHECK(gdp != gdp2);
                CHECK(gdp2 != gdp);
                CHECK(!(gdp == gdp2));
                CHECK(!(gdp2 == gdp));
            }
        }
    }
}
