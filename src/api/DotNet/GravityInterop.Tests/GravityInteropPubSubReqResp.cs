using gravity_wrapper;
using GravityInterop;
using System.Collections.Concurrent;
using System.Diagnostics;
using Google.Protobuf;

namespace GravityInterop.Tests
{
        [TestFixture]
        public class PubSubTests
        {
            private GravityNode GN;
            private GravitySubscriber m_subscriber;
            private BlockingCollection<Action> m_pubSubactionQueue;
            private Action<GravityDataProduct> m_pubSubAction;
            
            private static readonly string PubSubId = "PubSubTestDP";

            [SetUp]
            public void Setup()
            {
                Console.WriteLine("StartPubSub");
                GN = new GravityNode("PubSubTest");

                m_pubSubactionQueue = new BlockingCollection<Action>();
                m_subscriber = new GravitySubscriber((dps) =>
                {
                    foreach (var dp in dps)
                    {
                        //Queue the dataproduct for handling later
                        m_pubSubactionQueue.Add(() => m_pubSubAction(dp));
                    }
                });
                GN.RegisterDataProduct(PubSubId, GravityInterop.GravityTransportTypes.Types.TCP);
                GN.Subscribe(PubSubId, m_subscriber);
            }

            [Test]
            public void RunPubSub()
            {
                int X = 0, Y = 0;
                List<(float, float)> xys = new List<(float, float)>();
                int out_cnt = 0;

                //Executed for each data product after the publishing is complete
                m_pubSubAction = (dp) =>
                {
                    var ji = gravity_wrapper.JoystickInput.Parser.ParseFrom(dp.getBytes());
                    Assert.IsTrue(ji.X == xys[out_cnt].Item1 && ji.Y == xys[out_cnt].Item2);
                    out_cnt++;
                };

                //Publish 10 JoystickInputs
                for (int i = 0; i < 10; i++)
                {
                    gravity_wrapper.JoystickInput ji = new JoystickInput();
                    ji.X = X++;
                    ji.Y = Y++;
                    xys.Add((ji.X, ji.Y));

                    GravityDataProduct newDp = new GravityDataProduct(PubSubId);
                    byte[] pbBytes = new byte[ji.CalculateSize()];
                    ji.WriteTo((Span<byte>)pbBytes);
                    newDp.setFromBytes(pbBytes);
                    GN.Publish(newDp);
                    System.Console.WriteLine("Published a DP");
                }

                //Process each data product, blocking if it hits an empty condition
                foreach (var action in m_pubSubactionQueue.GetConsumingEnumerable())
                {
                    action();
                    if (out_cnt == 10)
                        break;
                }

                Assert.Pass("PubSub test passed");
            }
        }

    [TestFixture]
    public class ReqRespTests
    {
        private GravityNode GN;
        private BlockingCollection<Action> m_reqRespActionQueue;

        private GravityServiceProvider m_gravityServiceProvider;
        private GravityServiceRequestor m_gravityServiceRequestor;

        private static readonly string ServiceId = "MultiplicationService";

        [SetUp]
        public void Setup()
        {
            Console.WriteLine("StartReqResp");
            GN = new GravityNode("ReqRespTest");

            m_reqRespActionQueue = new BlockingCollection<Action>();
            m_gravityServiceProvider = new GravityServiceProvider((id, dp) =>
            {
                //Use the JoystickInput as a test PB to just multiply it's X and Y values
                var ji = gravity_wrapper.JoystickInput.Parser.ParseFrom(dp.getBytes());
                Console.WriteLine($"received XY to multiply from gravity of {ji.X} {ji.Y}");
                GravityDataProduct newDp = new GravityDataProduct("MultResponse");
                gravity_wrapper.MultiplicationResponse mrsp = new gravity_wrapper.MultiplicationResponse();
                mrsp.Result = ji.X * ji.Y;
                byte[] pbBytes = new byte[mrsp.CalculateSize()];
                mrsp.WriteTo((Span<byte>)pbBytes);
                newDp.setFromBytes(pbBytes);
                return newDp;
            });

            GN.RegisterServiceProvider(ServiceId, GravityInterop.GravityTransportTypes.Types.TCP, m_gravityServiceProvider);
        }

        [Test]
        public void RunReqResp()
        {
            int X = 0, Y = 0;
            List<(float, float)> xys = new List<(float, float)>();
            int cnt = 0;

            m_gravityServiceRequestor = new GravityServiceRequestor((serviceId, requestId, dp) =>
            {
                //Queue the checks until after all the requests are made
                m_reqRespActionQueue.Add(() =>
                {
                    var mr = gravity_wrapper.MultiplicationResponse.Parser.ParseFrom(dp.getBytes());
                    Console.WriteLine($"Received service response of {mr.Result}");

                    Assert.IsTrue(mr.Result == xys[cnt].Item1 * xys[cnt].Item2);
                    cnt++;
                });
            });
            for (int i = 0; i < 10; i++)
            {
                GravityDataProduct newDp = new GravityDataProduct(ServiceId);
                gravity_wrapper.JoystickInput ji = new gravity_wrapper.JoystickInput();
                ji.X = X++;
                ji.Y = Y++;
                xys.Add((ji.X, ji.Y));

                byte[] pbBytes = new byte[ji.CalculateSize()];
                ji.WriteTo((Span<byte>)pbBytes);
                newDp.setFromBytes(pbBytes);
                Console.WriteLine($"Requested:" + $"{ji.X}x{ji.Y}");
                GN.ServiceRequestAsync(m_gravityServiceRequestor, ServiceId, $"{ji.X}x{ji.Y}", newDp);
            }

            //Process the checks, blocking if it hits an empty condition
            foreach (var action in m_reqRespActionQueue.GetConsumingEnumerable())
            {
                action();
                if (cnt == 10)
                    break;
            }

            Assert.Pass("ReqResp test passed");
        }
    }
}