#include "GravityArchiver.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //Smaller include
#include <windows.h> //For Sleep
#define sleep Sleep
#endif

int main(int argc, char** argv)
{
  using namespace gravity;
  GravityNode gn;
  gn.init();
  gravity::Archiver arch(&gn, "test", "test", "root", "bamas");
  arch.start();

  while(true)
  {
    sleep(4294967295u); //Sleep for as long as we can.  (We can't join on the Subscription Manager thread).
  }

}
