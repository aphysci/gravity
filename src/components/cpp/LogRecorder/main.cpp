#include <iostream>
#include <GravityNode.h>
#include "GravityLogRecorder.h"

int main()
{
  using namespace gravity;
  GravityNode gn;
  gn.init();

  LogRecorder lr(&gn, "MyBase");

  lr.start();
  pause();
}
