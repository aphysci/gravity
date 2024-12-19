#include "Utility.h"
#include "../doctest.h"
#include "../utils/utils.h"

#include <climits>
#include <cfloat>
#include <string>
#include <iostream>

using namespace gravity;

TEST_CASE("String helper tests") {
  
  GIVEN("an empty string") {
    std::string empty = "";
    THEN("return an empty string") {
      CHECK(empty==StringToLowerCase(empty));
      CHECK(empty==StringCopyToLowerCase(empty));
      CHECK(empty==trim(empty));
    }
    THEN("return default value") {
      int i = 423;
      CHECK(i == StringToInt(empty,i));
      double dbl = 24.23;
      CHECK(doctest::Approx(dbl) == StringToDouble(empty,dbl));
    }
  }

  SUBCASE("test *ToLowerCase() functions") {

    GIVEN("A single uppercase letter.") {
      char c[] = {'A'};
      std::string s = "A";
      std::string expected = "a";
      THEN("return lower case character") {
        CHECK(expected == StringToLowerCase(s));
        CHECK(expected == StringCopyToLowerCase(s));
        CHECK(expected[0] == StringToLowerCase(c, 1)[0]);
      }
      THEN("StringToLower additionally makes in-place modification") {
        StringToLowerCase(c, 1);
        CHECK(expected[0] == c[0]);
      }
    } 

    GIVEN("A single lowercase letter.") {
      char c[] = {'a'};
      std::string s = "a";
      std::string expected = "a";
      THEN("return lower case character") {
        CHECK(expected == StringToLowerCase(s));
        CHECK(expected == StringCopyToLowerCase(s));
        CHECK(expected[0] == StringToLowerCase(c, 1)[0]);
      }
      THEN("StringToLower makes additionally in-place modification") {
        StringToLowerCase(c, 1);
        CHECK(expected[0] == c[0]);
      }
    }

    GIVEN("A mixed case word.") {
      char c[] = "hEllO WorlD";
      std::string s = "hEllO WorlD"; 
      int sz = s.size();
      std::string expected = "hello world";
      THEN("return the same sequence") {
        CHECK(expected == StringToLowerCase(s));
        CHECK(expected == StringCopyToLowerCase(s));
        check_equality(expected, StringToLowerCase(c, sz), sz);
      }
      THEN("StringToLower makes additional in-place modifications") {
        StringToLowerCase(c, sz);
        check_equality(expected, c, sz);
      }
    }

    GIVEN("A sequence of alpha and non-alpha characters.") {
      char c[] = "2R#(_. g";
      std::string s = "2R#(_. g"; 
      int sz = s.size();
      std::string expected = "2r#(_. g";
      THEN("return the same sequence") {
        CHECK(expected == StringToLowerCase(s));
        CHECK(expected == StringCopyToLowerCase(s));
        check_equality(expected, StringToLowerCase(c, sz), sz);
      }
      THEN("StringToLower makes no additional in-place modifications") {
        StringToLowerCase(c, sz);
        check_equality(expected, c, sz);
      }
    }

  }


  SUBCASE("test StringToDouble() and StringToInt() functions") {
    int default_int = 123;
    double default_double = 45.67;

    GIVEN("An invalid string") {
      std::string s = "hello";
      THEN("return default value") {
        CHECK(default_int == StringToInt(s, default_int));
        CHECK(doctest::Approx(default_double) == StringToDouble(s, default_double));
      }
    }

    GIVEN("A double") {
      std::string s = "3.82";
      double expected_dbl = 3.82;
      int expected_int = 3;
      THEN("return truncated double") {
        CHECK(expected_int == StringToInt(s, default_int));
      }
      THEN("return double") {
        CHECK(doctest::Approx(expected_dbl) == StringToDouble(s, default_double));
      }
    }

    GIVEN("A negative double") {
      std::string s = "-3.82";
      double expected_dbl = -3.82;
      int expected_int = -3;
      THEN("return truncated double") {
        CHECK(expected_int == StringToInt(s, default_int));
      }
      THEN("return double") {
        CHECK(doctest::Approx(expected_dbl) == StringToDouble(s, default_double));
      }
    }

    GIVEN("The maximum value for an integer.") {      
      std::string s = std::to_string(INT_MAX);
      THEN("return this integer") {
        CHECK(INT_MAX == StringToInt(s, default_int));
      }
    }

    GIVEN("An out of limits integer.") {      
      std::string s = std::to_string(INT_MAX) + '0'; //guaranteed out-of-bounds
      THEN("return the default integer") {
        CHECK(default_int == StringToInt(s, default_int));
      }
    }

    GIVEN("The maximum value for a double.") {      
      std::string s = std::to_string(DBL_MAX);
      THEN("return this double") {
        CHECK(DBL_MAX == StringToDouble(s, default_double));
      }
    }

    GIVEN("An out of limits double.") {      
      std::string s = '1' + std::to_string(DBL_MAX); //guaranteed out-of-bounds
      THEN("return the default double") {
        CHECK(default_double == StringToDouble(s, default_double));
      }
    }
  }

  SUBCASE("test trim() function") {
    GIVEN("an empty string") {
      std::string str = "";
      THEN("return an empty string") {
        CHECK(trim(str) == "");
        CHECK(str == "");
      }
    }

    GIVEN("A string with default deliminators") {
      std::string str = "deliminators: \f\n\r\t\v";
      THEN("modify and return a string with deliminators removed") {
        CHECK(trim(str) == "deliminators:");
        CHECK(str == "deliminators:");
      }
    }

    GIVEN("A string") {
      std::string str = "my string";
      THEN("modify and return a string with my deliminators removed") {
        CHECK(trim(str, "mg") == "y strin");
        CHECK(str == "y strin");
      }
    }
  }

  SUBCASE("test replaceAll() function") {

    GIVEN("an empty string") {
      std::string str = "";
      std::string oldValue = "\n";
      std::string newValue = "\t";
      THEN("string remains empty") {
        replaceAll(str, oldValue, newValue);
        CHECK(str == "");
      } 
    }

    GIVEN("an empty string for the old value to replace") {
      std::string str = "my unique string.";
      std::string oldValue = "";
      std::string newValue = "a";
      THEN("do not modify string") {
        replaceAll(str, oldValue, newValue);
        CHECK(str == "my unique string.");
      } 
    }

    GIVEN("an empty string for the new value to replace") {
      std::string str = "my unique string named str.";
      std::string oldValue = "str";
      std::string newValue = "";
      THEN("modify string with removed values") {
        replaceAll(str, oldValue, newValue);
        CHECK(str == "my unique ing named .");
      } 
    }

  }

}
