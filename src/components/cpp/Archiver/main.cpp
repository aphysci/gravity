#include "GravityArchiver.h"

int main(int argc, char** argv)
{
  using namespace gravity;
  GravityNode gn;
  gravity::Archiver arch(&gn, "test", "test", "root", "bamas");
  arch.start();

  while(true)
  {
    sleep(4294967295); //Sleep for as long as we can.  (We can't join on the Subscription Manager thread).  
  }

}
