#include "CommUtil.h"
#include "GravityDataProduct.h"
#include "../doctest.h"
#include "zmq.h"
#include "BasicCounterDataProduct.pb.h"

#include <string>
#include <iostream>

using namespace gravity;

TEST_CASE("tests for sending/reading ZMQ messages") {
  
  //initialize zmq context
  void* context = zmq_init(1);
  REQUIRE_MESSAGE(context, "error initializing context");

  //create client socket and bind to endpoint
  void* clientSocket = zmq_socket(context, ZMQ_REQ);
  int ret = zmq_bind(clientSocket, "inproc://endpoint_for_accepting_connections");
  REQUIRE_MESSAGE(ret == 0, "error connecting client socket");

  //create service socket and bind to endpoint
  void* serviceSocket = zmq_socket(context, ZMQ_REP);
  ret = zmq_connect(serviceSocket, "inproc://endpoint_for_accepting_connections");
  REQUIRE_MESSAGE(ret == 0, "error connecting service socket");

  SUBCASE("tests for sending/receiving one-part String messages") {
    std::string message = "message";

    GIVEN("an empty string sent from client -> service") {
      std::string empty = "";
      sendStringMessage(clientSocket, empty, ZMQ_DONTWAIT);

      THEN("receive an empty message back") {
        std::string result = readStringMessage(serviceSocket,ZMQ_DONTWAIT);
        CHECK("" == result);
      }

      THEN("receive an empty message back") {
        std::string result = readStringMessage(serviceSocket); //without flag
        CHECK("" == result);
      }

    }

    GIVEN("a non-empty string sent from the service -> client") {
      sendStringMessage(serviceSocket, message, ZMQ_DONTWAIT);

      THEN("receive an empty message back") {
        std::string result = readStringMessage(clientSocket,ZMQ_DONTWAIT);
        CHECK("" == result);
      }

      THEN("receive an empty message back") {
        std::string result = readStringMessage(clientSocket); //without flag
        CHECK("" == result);
      }

    }  

    GIVEN("an incomplete message sent from the client -> service") {
      //Flag ZMQ_SNDMORE indicates that at least one more message follows 
      sendStringMessage(clientSocket, message, ZMQ_SNDMORE);
      THEN("receive an empty message back") {
        //Flag ZMQ_DONTWAIT indicates for this read function to be non-blocking 
        //It should return an empty string because it hasn't received any messages yet
        //Without this flag, this function would hang until indefinitely waiting for
        // a message that will never arrive.
        std::string result = readStringMessage(serviceSocket,ZMQ_DONTWAIT);
        CHECK("" == result);
      }
    }

    //check typical use case of request & response
    GIVEN("a non-empty string message sent from the client -> service") {
      sendStringMessage(clientSocket, message, ZMQ_DONTWAIT);

      THEN("receive message on service socket") {
        std::string result = readStringMessage(serviceSocket,ZMQ_DONTWAIT);
        CHECK(message == result);
        THEN("should be no more additional messages to read") {
          result = readStringMessage(serviceSocket,ZMQ_DONTWAIT);
          CHECK("" == result);
        }
      }

      THEN("receive message on service socket") {
        std::string result = readStringMessage(serviceSocket); //without flag
        CHECK(message == result);
        THEN("should be no more additional messages to read") {
          result = readStringMessage(serviceSocket); //without flag
          CHECK("" == result);
        }
      }

      GIVEN("a one-part string response from service -> client") {
        readStringMessage(serviceSocket,ZMQ_DONTWAIT);
        std::string response = "message 3";
        sendStringMessage(serviceSocket, response, ZMQ_DONTWAIT);

        THEN("receive message on client socket") {
          std::string result = readStringMessage(clientSocket,ZMQ_DONTWAIT);
          CHECK(response == result);
          THEN("should be no more additional messages to read") {
            result = readStringMessage(clientSocket,ZMQ_DONTWAIT);
            CHECK("" == result);
          }
        }

        THEN("receive message on client socket") {
          std::string result = readStringMessage(clientSocket); //without flag
          CHECK(response == result);
          THEN("should be no more additional messages to read") {
            result = readStringMessage(clientSocket); //without flag
            CHECK("" == result);
          }
        }

      }
    }
  }

  //Only testing with readStringMessage(void*) which is a blocking call.
  //The caller will be blocked until it receives a message.
  //
  //The unit tests above highlight the difference
  //between readStringMessage(void*,int) and readStringMessage(void*) so
  //no need for further testing. 
  //
  SUBCASE("tests for sending/receiving multi-part String messages") {
 
    GIVEN("non- empty strings sent from service -> client") {
      std::string message = "message";
      sendStringMessage(serviceSocket, message, ZMQ_DONTWAIT);

      THEN("receive an empty message back") {
        std::string result = readStringMessage(clientSocket); 
        CHECK("" == result);
      }
    }


    //check typical use case of request & response
    GIVEN("sending multiple string messages form client -> service") {

      std::string message1 = "message1";
      std::string message2 = "message2";
      std::string message3 = "message3";
      
      sendStringMessage(clientSocket, message1, ZMQ_SNDMORE);
      sendStringMessage(clientSocket, message2, ZMQ_SNDMORE);
      sendStringMessage(clientSocket, message3, ZMQ_DONTWAIT);

      THEN("receive string messages on service socket") {
        std::string result = readStringMessage(serviceSocket);
        CHECK(message1 == result);
        result = readStringMessage(serviceSocket);
        CHECK(message2 == result);
        result = readStringMessage(serviceSocket);
        CHECK(message3 == result);
        result = readStringMessage(serviceSocket);
        CHECK("" == result); //no more messages

        GIVEN("sending a multi-part response from service -> client") {
          readStringMessage(serviceSocket);
          readStringMessage(serviceSocket);
          readStringMessage(serviceSocket);

          std::string response1 = "response1";
          std::string response2 = "response2"; 
          std::string response3 = "response3"; 

          sendStringMessage(serviceSocket, response1, ZMQ_SNDMORE);
          sendStringMessage(serviceSocket, response2, ZMQ_SNDMORE);
          sendStringMessage(serviceSocket, response3, ZMQ_DONTWAIT);

          THEN("receive response messages on client socket") {
            std::string result = readStringMessage(clientSocket);
            CHECK(response1 == result);
            result = readStringMessage(clientSocket);
            CHECK(response2 == result);
            result = readStringMessage(clientSocket);
            CHECK(response3 == result);
            result = readStringMessage(clientSocket);
            CHECK("" == result); //no more messages
          }
        }
      }
    }   
  }

  //functions for reading/sending int, unit32_t, uint64_t are
  //nearly identical, so only testing with int 
  SUBCASE("tests for sending/receiving one-part int messages") {
    int message = 25;

    //check typical use case of request & response
    GIVEN("an integer message sent from the client -> service") {
      sendIntMessage(clientSocket, message, ZMQ_DONTWAIT);

      THEN("receive message on service socket") {
        int result = readIntMessage(serviceSocket);
        CHECK(message == result);
      }

      GIVEN("a one-part string response from service -> client") {
        readIntMessage(serviceSocket);
        int response = 4;
        sendIntMessage(serviceSocket, response, ZMQ_DONTWAIT);

        THEN("receive message on client socket") {
          int result = readIntMessage(clientSocket);
          CHECK(response == result);
        }
      }
    }

  }

  SUBCASE("tests for sending GravityDataProducts") {
    std::string dataProductID = "TestID";
    //check typical use case of request & response
    GIVEN("sending a non-empty GravityDataProduct from client->service") {
		  GravityDataProduct gdp(dataProductID);
      std::string data = "string of data";
      gdp.setData((const void*) data.c_str(), data.size()); 
      int bytes = sendGravityDataProduct(clientSocket, gdp, ZMQ_DONTWAIT); 
      THEN("receive back the correct number of bytes") {
        CHECK(gdp.getSize() == bytes);
      }
    } 
  }

  SUBCASE("tests for sending protobufs") {
		BasicCounterDataProductPB pb;
		pb.set_count(34);

    GIVEN("sending a non-empty Protobuf from client->service") {
      int bytes = sendProtobufMessage(clientSocket, pb, ZMQ_DONTWAIT); 
      THEN("receive back the correct number of bytes") {
        CHECK(pb.ByteSize() == bytes);
      }
    } 
  }

  // SUBCASE("tests for timeval functions") {
  //   struct timeval {
  //    long tv_sec;
  //    long tv_usec;
  //   };
  //   GIVEN("a timeval") {
  //     timeval t1;
  //     t1.tv_sec = 100;
  //     t1.tv_usec = 1000;
  //     THEN("add seconds to timeval") {
  //       timeval t2 = addTime(&t1, 1); 
  //       CHECK(t2.tv_sec == 101);
  //       CHECK(t2.tv_usec == 1000);
  //     }
  //   }
  // }

  zmq_close(clientSocket);
  zmq_close(serviceSocket);
}
