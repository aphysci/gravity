#include <iostream>
#include <GravityNode.h>
#include "GravityLogRecorder.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define pause() while(true) Sleep(4000000000u);
#endif

int main()
{
  using namespace gravity;
  GravityNode gn;
  gn.init("GravityLogRecorder");

  LogRecorder lr(&gn, "MyBase");

  lr.start();

  gn.waitForExit();
}
