#include "GravityLogger.h"
#include "../doctest.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

using namespace gravity;

// Create a logger that uses the Logger interface
class TestLogger : public Logger {
  public: 
    TestLogger() = default;
    std::string lastMessage;

    //must implement the Log function   
    void Log(int level, const char* messagestr)
    {
      lastMessage = std::string(messagestr);
    }
};

TEST_CASE("Tests without the mocking framework") {

  SUBCASE("Test the LogLevelToString() function") {
    GIVEN("A LogLevel") {
      Log::LogLevel loglevel = Log::LogLevel::FATAL;
      THEN("Return as a string") {
         auto c_str = Log::LogLevelToString(loglevel); 
         std::string str(c_str);
         CHECK(str == "FATAL");
      }
    }
  }

  SUBCASE("Test the LogStringToLevel() function") {
    GIVEN("An empty string") {
      THEN("Return as LogLevel::NONE") {
        Log::LogLevel loglevel = Log::LogStringToLevel(""); 
        CHECK(loglevel == Log::LogLevel::NONE);
      }
    }

    GIVEN("An invalid string") {
      THEN("Return as LogLevel::NONE") {
        Log::LogLevel loglevel = Log::LogStringToLevel("error"); 
        CHECK(loglevel == Log::LogLevel::NONE);
      }
    }

    GIVEN("An lower-case string of a valid log level") {
      THEN("Return LogLevel") {
        Log::LogLevel loglevel = Log::LogStringToLevel("fatal"); 
        CHECK(loglevel == Log::LogLevel::FATAL);
      }
    }

  }

  SUBCASE("Test ability to truncate string") {
      GIVEN("a string with %n") {
          TestLogger* logger = new TestLogger();
          Log::initAndAddLogger(logger, Log::DEBUG); //ptr now owned by Log
          std::string truncStr(" !!!!!!!!! '%n' DETECTED, MESSAGE TRUNCATED");
          THEN("Truncate short log line") {
              std::string shortBadStr("a bad string %n");
              Log::debug(shortBadStr.c_str());
              CHECK(logger->lastMessage == shortBadStr.substr(0, shortBadStr.size() - 3) + truncStr);
          }
          THEN("Truncate long log line") {
              std::string longBadStr(600, 'a');
              longBadStr += "%n";
              Log::debug(longBadStr.c_str());
              CHECK(logger->lastMessage == longBadStr.substr(0, 512 - truncStr.size() - 2) + truncStr);
          }
          THEN("No truncate %n as arg") {
              Log::debug("good string %s", "%n");
              CHECK(logger->lastMessage == "good string %n");
          }
          Log::RemoveLogger(logger);
      }
  }
  //Note: equally tests initAndAddConsoleLogger because
  //the ConsoleLogger inherits from FileLogger with the only
  //difference being that logs are directed to stdout
  SUBCASE("Test the initAndAddFileLogger() function") {

    GIVEN("a debug file logger with an invalid componentID") {
      std::string componentID =  "/~]filename";
      Log::initAndAddFileLogger("", componentID.c_str(), Log::LogLevel::DEBUG);
      THEN("Create a Gravity.log file") {
        std::ifstream myfile("Gravity.log"); 
        CHECK(myfile.is_open());
        myfile.close();
        GIVEN("A debug log message") {
          std::string message = "My unique message";
          //log message
          Log::debug(message.c_str());
          THEN("Write log message with invalid componentID") {
            //read message
            std::string line;
            if(myfile.is_open())
            {
              std::getline(myfile, line);
              auto pos = line.find_first_of("DEBUG");
              CHECK_MESSAGE(pos != std::string::npos, "contains DEBUG statement");
              pos = line.find_first_of(componentID);
              CHECK_MESSAGE(pos != std::string::npos, "contains componentID");
              pos = line.find_first_of(message);
              CHECK_MESSAGE(pos != std::string::npos, "contains message");
              CHECK_MESSAGE(!(std::getline(myfile,line)), "Only one line should have been written");
              myfile.close();
            }
            CHECK(1 == Log::NumberOfLoggers());
            Log::CloseLoggers();
            CHECK(0 == Log::NumberOfLoggers());
            std::remove("Gravity.log");
          }
        }
      }
      
    }

    GIVEN("an initialized file logger with log level Debug") {
      std::string componentID = "TestID"; 
      //if file exists, delete it
      std::string path = componentID + ".log";
      Log::initAndAddFileLogger("", componentID.c_str(), Log::LogLevel::DEBUG);

      GIVEN("a debug Log message") {
        std::string message = "My unique message";
        //log message
        Log::debug(message.c_str());
        THEN("write debug message to the file") {
          //read message
          std::string line;
          std::ifstream myfile(path); 
          CHECK(myfile.is_open());
          if(myfile.is_open())
          {
            std::getline(myfile, line);
            auto pos = line.find_first_of("DEBUG");
            CHECK_MESSAGE(pos != std::string::npos, "contains DEBUG statement");
            pos = line.find_first_of(componentID);
            CHECK_MESSAGE(pos != std::string::npos, "contains componentID");
            pos = line.find_first_of(message);
            CHECK_MESSAGE(pos != std::string::npos, "contains message");
            CHECK_MESSAGE(!(std::getline(myfile,line)), "Only one line should have been written");
            myfile.close();
          }
        }
      }

      GIVEN("a trace Log message") {
        std::string message = "My unique message";
        //log message
        Log::trace(message.c_str());
        THEN("create an empty file without writing trace message") {
          //read message
          std::string line;
          std::ifstream myfile(path); 
          CHECK(myfile.is_open());
          if(myfile.is_open())
          {
            CHECK_MESSAGE(!(std::getline(myfile,line)), "Nothing should have been written");
            myfile.close();
          }
        }
      }


      GIVEN("an empty debug Log message") {
        std::string message = "";
        //log message
        Log::debug(message.c_str());
        THEN("write an empty debug message to the file") {
          //read message
          std::string line;
          std::ifstream myfile(path); 
          CHECK(myfile.is_open());
          if(myfile.is_open())
          {
            std::getline(myfile, line);
            auto pos = line.find_first_of("DEBUG");
            CHECK_MESSAGE(pos != std::string::npos, "contains DEBUG statement");
            pos = line.find_first_of(componentID);
            CHECK_MESSAGE(pos != std::string::npos, "contains componentID");
            pos = line.find_first_of(message);
            CHECK_MESSAGE(pos == std::string::npos, "contains message");
            CHECK_MESSAGE(!(std::getline(myfile,line)), "Only one line should have been written");
            myfile.close();
          }
        }
      }
      CHECK(1 == Log::NumberOfLoggers());
      Log::CloseLoggers();
      CHECK(0 == Log::NumberOfLoggers());
      std::remove(path.c_str());
    }
  }
  
  SUBCASE("test the CloseLoggers() and RemoveLogger() functions") {

    GIVEN("initializing 2 loggers through the init functions") {
      Log::initAndAddFileLogger("", "filename", Log::LogLevel::DEBUG);
      Log::initAndAddFileLogger("", "filename2", Log::LogLevel::DEBUG);
      THEN("successfully close loggers with CloseLoggers()") {
        CHECK(2 == Log::NumberOfLoggers());
        Log::CloseLoggers();
        CHECK(0 == Log::NumberOfLoggers());
      }
    }

    GIVEN("manually create and add loggers") {
      TestLogger* logger = new TestLogger();
      Log::initAndAddLogger(logger, Log::DEBUG); //ptr now owned by Log
      TestLogger* logger2 = new TestLogger();
      Log::initAndAddLogger(logger2, Log::DEBUG); //ptr now owned by Log

      THEN("successfully close loggers with RemoveLogger()") {
        CHECK(2 == Log::NumberOfLoggers());
        Log::RemoveLogger(logger2); //remove specific Loggger
        Log::RemoveLogger(logger); //remove specific Loggger
        CHECK(0 == Log::NumberOfLoggers());
      }

      THEN("successfully close loggers with CloseLoggers()") {
        CHECK(2 == Log::NumberOfLoggers());
        Log::CloseLoggers();
        CHECK(0 == Log::NumberOfLoggers());
      }
    }

    GIVEN("add pointer to same logger more than once") {
      TestLogger* logger = new TestLogger();
      TestLogger* logger2 = new TestLogger();
      Log::initAndAddLogger(logger, Log::DEBUG); //ptr now owned by Log
      Log::initAndAddLogger(logger2, Log::DEBUG); //ptr now owned by Log
      Log::initAndAddLogger(logger, Log::DEBUG); //duplicate ptr

      THEN("do not store duplicate loggers and close loggers with CloseLoggers") {
        CHECK(2 == Log::NumberOfLoggers());
        Log::CloseLoggers();
        CHECK(0 == Log::NumberOfLoggers());
      }
     
      THEN("do not store duplicate loggers and close loggers with RemoveLoggers") {
        CHECK(2 == Log::NumberOfLoggers());
        Log::RemoveLogger(logger2); //remove specific Loggger
        Log::RemoveLogger(logger); //remove specific Loggger
        CHECK(0 == Log::NumberOfLoggers());
      }

    }
  }
} 

