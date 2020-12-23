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
        int pintp = gn.getIntParam("pintp", 0);
        CHECK(pint1 == 1);
        CHECK(pint2 == 2);
        CHECK(pint3 == 3);
        CHECK(pint4 == 4);
        CHECK(pintp == 1);

        int nint1 = gn.getIntParam("nint1", 0);
        CHECK(nint1 == -9);

        double pflt1 = gn.getFloatParam("pflt1", 0);
        double pflt2 = gn.getFloatParam("pflt2", 0);
        double pflt3 = gn.getFloatParam("pflt3", 0);
        double pflt4 = gn.getFloatParam("pflt4", 0);
        double pfltp = gn.getFloatParam("pfltp", 0);
        CHECK(pflt1 == 1.6);
        CHECK(pflt2 == 2.6);
        CHECK(pflt3 == 3.6);
        CHECK(pflt4 == 4.6);
        CHECK(pfltp == 9.7);

        double nflt1 = gn.getFloatParam("nflt1", 0);
        CHECK(nflt1 == -9.6);
        

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
        std::string nflt1 = gn.getStringParam("nflt1", "default");
        CHECK(pint1 == "1");
        CHECK(pflt1 == "1.6");
        CHECK(bool1 == "true");
        CHECK(nflt1 == "-9.6");
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

  SUBCASE("Testing uninitalization GravideNode")
  {
    class TestStub : public GravitySubscriber, public GravityRequestor, public GravityServiceProvider,
                     public GravityHeartbeatListener, public GravitySubscriptionMonitor
    {
    public:
      GRAVITY_API virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts) {}
      GRAVITY_API virtual void requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response) {}
      virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct)
      {
          return std::shared_ptr<GravityDataProduct>(NULL);
      }
      GRAVITY_API virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds) {}
      GRAVITY_API virtual void MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds) {}
      GRAVITY_API virtual void subscriptionTimeout(std::string dataProductID, int milliSecondsSinceLast, std::string filter, std::string domain) {}
      virtual ~TestStub() {}
    };

    static GravityNode gn;

    GIVEN("an uninitialized GravityNode") {

      THEN("getParam methods should return default value") {
        std::string string1 = gn.getStringParam("string1", "default");
        CHECK(string1 == "default");
        int int1 = gn.getIntParam("int1", -123);
        CHECK(int1 == -123);
        float float1 = gn.getFloatParam("float1", 1.234);
        CHECK(float1 == 1.234f);
        bool bool1 = gn.getBoolParam("bool1", true);
        CHECK(bool1);
      }

      THEN("methods should return NOT_INITIALIZED") {
        GravityReturnCode ret;

        ret = gn.subscribe("", TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.subscribe("", TestStub(), "");
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.subscribe("", TestStub(), "", "");
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.subscribe("", TestStub(), "", "", false);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.unsubscribe("", TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.publish(GravityDataProduct());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.request("", GravityDataProduct(), TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.startHeartbeat(100);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.stopHeartbeat();
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.registerDataProduct("", GravityTransportType::TCP);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.registerDataProduct("", GravityTransportType::TCP, true);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.unregisterDataProduct("");
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.registerService("", GravityTransportType::TCP, TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.unregisterService("");
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.registerHeartbeatListener("", 100, TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.unregisterHeartbeatListener("", "");
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.registerRelay("", TestStub(), true, GravityTransportType::TCP);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.registerRelay("", TestStub(), true, GravityTransportType::TCP, true);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.unregisterRelay("", TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);

        ret = gn.setSubscriptionTimeoutMonitor("", TestStub(), 100);
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
        ret = gn.clearSubscriptionTimeoutMonitor("", TestStub());
        CHECK(ret == GravityReturnCodes::NOT_INITIALIZED);
      }

      THEN("other methods should return as expected") {
        std::shared_ptr<GravityDataProduct> retSP = gn.request("", GravityDataProduct());
        CHECK(!retSP);

        CHECK(gn.getComponentID() == "");
        CHECK(gn.getIP() == "");
        CHECK(gn.getDomain() == "");

        std::shared_ptr<FutureResponse> ret = gn.createFutureResponse();
        CHECK(!ret);
      }
    }
  }
}
