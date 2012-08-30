#include <GravityNode.h>

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
