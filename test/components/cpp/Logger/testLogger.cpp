//class GravityNode {};
#include <iostream>
#include "GravityLogger.h"

int main()
{
  using namespace gravity;
  GravityNode gn;
  gn.init("TestLogger");

  cout << "test" << endl;

  //Log::setLocalLevel(Log::TRACE);
  //Log::init(gn, "test.log", 43211);

  getchar();

  cout << "finished init" << endl;

  Log::trace("Trace");

  Log::debug("1+1=%d", (1+1));

  //  OtherFunc();

  Log::message("Hi %s", "there");

  Log::warning("Warn!!!");

  Log::critical("1!=2");

  Log::fatal("AAAAAaaaaaaahhhhhh....");

}
