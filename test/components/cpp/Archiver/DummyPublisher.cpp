#include <GravityNode.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //Smaller include
#include <windows.h> //For Sleep
#define sleep Sleep
#endif

int main(int argc, char** argv)
{
  using namespace gravity;
  char* myDataProductID = "DummyDataProduct";

  if(argc > 1)
    myDataProductID = argv[1];

  GravityNode gn;
  gn.init();
  gn.registerDataProduct(myDataProductID, 54321, "tcp");

  GravityDataProduct dataProduct(myDataProductID);
  int count = 0;

  while(true)
  {
    dataProduct.setData(&count, sizeof(count));

    gn.publish(dataProduct);

    sleep(1);

    count++;
    count = count % 20;
  }

  gn.unregisterDataProduct(myDataProductID);

}
