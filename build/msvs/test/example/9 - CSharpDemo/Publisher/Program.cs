using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Publisher
{
    class Program
    {
        static void Main(string[] args)
        {
            System.Console.Out.WriteLine("Started C# Publisher");
            GravityCS.GravityInteractor g = new GravityCS.GravityInteractor();
            g.Init(".Net Gravity Test Publisher");
            System.Console.Out.WriteLine("Gravity Initialized");

            g.RegisterDataProduct("CSTestDataProduct", GravityCS.GravityTransportType.TCP );
            System.Console.Out.WriteLine("Registered CSTestDataProduct");

            g.RegisterService("CSTestRequest", GravityCS.GravityTransportType.TCP, delegate(String serviceID, GravityCS.DataProduct dataProducts)
            {
                MultiplicationOperandsPB.Builder multRequest = MultiplicationOperandsPB.CreateBuilder();
                dataProducts.getProtobufObject(multRequest);

                MultiplicationResultPB.Builder multResult = MultiplicationResultPB.CreateBuilder();
                multResult.Result = multRequest.MultiplicandA * multRequest.MultiplicandB; //Do the calculation

                GravityCS.DataProduct result = new GravityCS.DataProduct("MultResult");
                result.setData(multResult.Build());

                return result;
            });
            System.Console.Out.WriteLine("Registered CSTestRequest");
            
            bool quit = false;
            int value = 0;
            while (!quit)
            {
                GravityCS.DataProduct testDP = new GravityCS.DataProduct("CSTestDataProduct");
                //testDP.setData();
                BasicCounterDataProductPB.Builder proto = BasicCounterDataProductPB.CreateBuilder();
                value++;
                proto.Count = value;

                testDP.setData(proto.Build());
                g.Publish(testDP);

                System.Console.Out.WriteLine("Published Data Product");

                System.Threading.Thread.Sleep(1000);
            }

            g.UnregisterDataProduct("CSTestDataProduct");
        }
    }
}
