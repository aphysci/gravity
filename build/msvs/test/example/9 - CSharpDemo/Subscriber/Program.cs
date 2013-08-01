using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Subscriber
{
    class Program
    {
        static void Main(string[] args)
        {
            GravityCS.GravityInteractor g = new GravityCS.GravityInteractor();
            g.Init(".Net Gravity Test Subscriber");

            g.Subscribe("CSTestDataProduct", delegate(IList<GravityCS.DataProduct> dataProducts) {
                foreach (GravityCS.DataProduct d in dataProducts)
                {
                    BasicCounterDataProductPB.Builder counterPB = BasicCounterDataProductPB.CreateBuilder();
                    d.getProtobufObject(counterPB);
                    System.Console.Out.WriteLine("Count: {0}", counterPB.Count);
                }
            });

            bool quit = false;
            Random rand = new Random();
            while (!quit)
            {
                //Let's try out making requests
                MultiplicationOperandsPB.Builder multRequest = MultiplicationOperandsPB.CreateBuilder();
                multRequest.MultiplicandA = rand.Next(10);
                multRequest.MultiplicandB = rand.Next(10);

                GravityCS.DataProduct dp = new GravityCS.DataProduct("CSTestRequest");
                dp.setData(multRequest.Build());
                g.Request("CSTestRequest", dp, delegate(String serviceID, String requestID, GravityCS.DataProduct dataProduct)
                {
                    MultiplicationResultPB.Builder multResult = MultiplicationResultPB.CreateBuilder();
                    dataProduct.getProtobufObject(multResult);
                    System.Console.Out.WriteLine("{0} x {1} = {2}", multRequest.MultiplicandA, multRequest.MultiplicandB, multResult.Result);
                });

                System.Threading.Thread.Sleep(1000);
            }

        }


    }
}
