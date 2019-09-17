#include "GravityNode.h"
#include "Utility.h"
#include "../doctest.h"

#include <string>
#include <iostream>

using namespace gravity;

TEST_CASE("Tests without the mocking framework") {

  GIVEN("A GravityReturnCode") {
    THEN("return that code in a string.") {
      GravityNode gravityNode;
      CHECK("SUCCESS" == gravityNode.getCodeString(GravityReturnCode::SUCCESS));
    }
  }

  //
  // Note that parsing method is different on different systems.
  // Do we have to conduct tests on different machines to validate system?
  // 
 
  SUBCASE("Testing the Gravity.ini parsing functions")
  {
    //only initializing gravity node once - will make it quicker to run
    static GravityNode gn;
    static bool init = true;
    if (init)
      gn.init("Test");
    init = false;  

    GIVEN("a Gravity.ini file") {

      THEN("Different spacing shouldn't affect keys and values") {
        std::string string1 = gn.getStringParam("string1", "default");
        std::string string2 = gn.getStringParam("string2", "default");
        std::string string3 = gn.getStringParam("string3", "default");
        std::string string4 = gn.getStringParam("string4", "default");
        CHECK(string1 == "message1");
        CHECK(string2 == "message2");
        CHECK(string3 == "message3");
        CHECK(string4 == "message4");

        int pint1 = gn.getIntParam("pint1", 0);
        int pint2 = gn.getIntParam("pint2", 0);
        int pint3 = gn.getIntParam("pint3", 0);
        int pint4 = gn.getIntParam("pint4", 0);
        CHECK(pint1 == 1);
        CHECK(pint2 == 2);
        CHECK(pint3 == 3);
        CHECK(pint4 == 4);

        double pflt1 = gn.getFloatParam("pflt1", 0);
        double pflt2 = gn.getFloatParam("pflt2", 0);
        double pflt3 = gn.getFloatParam("pflt3", 0);
        double pflt4 = gn.getFloatParam("pflt4", 0);
        CHECK(pflt1 == 1.6);
        CHECK(pflt2 == 2.6);
        CHECK(pflt3 == 3.6);
        CHECK(pflt4 == 4.6);

        bool bool1 = gn.getBoolParam("bool1", false);
        bool bool2 = gn.getBoolParam("bool2", false);
        bool bool3 = gn.getBoolParam("bool3", false);
        bool bool4 = gn.getBoolParam("bool4", false);
        CHECK(bool1 == true);
        CHECK(bool2 == true);
        CHECK(bool3 == true);
        CHECK(bool4 == true);
      }

      THEN("calling keys is not case-sensitive") {
        std::string STRING1 = gn.getStringParam("STRING1", "default");
        CHECK(STRING1 == "message1");
        int PINT1 = gn.getIntParam("PINT1", 0);
        CHECK(PINT1 == 1);
      }

      THEN("the same key but with different case should not overwrite value") {
        std::string string6 = gn.getStringParam("string6", "default");
        std::string String6 = gn.getStringParam("String6", "default");
        CHECK_MESSAGE(string6 == "message6", "value was overwritten");
        CHECK_MESSAGE(String6 == "message6", "value was overwritten");

        int pint6 = gn.getIntParam("pint6", 0);
        int Pint6 = gn.getIntParam("Pint6", 0);
        CHECK(pint6 == 6);
        CHECK(Pint6 == 6);

        double pflt6 = gn.getFloatParam("pflt6", 0);
        double Pflt6 = gn.getFloatParam("Pflt6", 0);
        CHECK(pflt6 == 6.6);
        CHECK(Pflt6 == 6.6);

        bool bool6 = gn.getBoolParam("bool6", false);
        bool Bool6 = gn.getBoolParam("Bool6", false);
        CHECK(bool6 == true);
        CHECK(Bool6 == true);
      }
      
      THEN("comment on the same line will not affect parsing") {
        std::string string5 = gn.getStringParam("string5", "default");
        CHECK(string5 == "message5");
        int pint5 = gn.getIntParam("pint5", 0);
        CHECK(pint5 == 5);
        double pflt5 = gn.getFloatParam("pflt5", 0);
        CHECK(pflt5 == 5.6);
        bool bool5 = gn.getBoolParam("bool5", false);
        CHECK(bool5 == true);
      }

      THEN("a very large integer should return the default") {
        int pintLarge = gn.getIntParam("pintLarge", 0);
        CHECK(pintLarge == 0);
      }

      THEN("can read an int, flt, or bool as a string") {
        std::string pint1 = gn.getStringParam("pint1", "default");
        std::string pflt1 = gn.getStringParam("pflt1", "default");
        std::string bool1 = gn.getStringParam("bool1", "default");
        CHECK(pint1 == "1");
        CHECK(pflt1 == "1.6");
        CHECK(bool1 == "true");
      }

      THEN("return default for an invalid int") {
        int readAsInt = gn.getIntParam("string1", 0);
        CHECK(readAsInt == 0);
      }

      THEN("using getIntParam on a float will truncate the float") {
        double pflt5 = gn.getIntParam("pflt5", 0);
        CHECK(pflt5 == 5.0);
      }
      
      THEN("boolean key is not case-sensitive and yes/y/t also indicate true") {
        bool bool7 = gn.getBoolParam("bool7", false);
        bool bool8 = gn.getBoolParam("bool8", false);
        bool bool9 = gn.getBoolParam("bool9", false);
        CHECK(bool7 == true);
        CHECK(bool8 == true);
        CHECK(bool9 == true);
      }
      
    }
    std::remove("Test.log");
  }
}
