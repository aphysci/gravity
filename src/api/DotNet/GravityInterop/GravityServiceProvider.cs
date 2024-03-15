using System;
using System.Diagnostics;
using static GravityInterop.NativeMethods;

namespace GravityInterop
{
    public class GravityServiceProvider : GravityInterop.Base.Object
    {

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private service_request m_callback;

        internal static IntPtr Create()
        {
            return NativeMethods.gravity_create_serviceprovider();
        }

        public GravityServiceProvider(GravityNode.Request request) : base(Create(), NativeMethods.gravity_delete_serviceprovider)
        {
            service_request req2 = (serviceId, dp) =>
            {
                GravityDataProduct request_gdp = new GravityDataProduct(dp);
                GravityDataProduct? response_gdp = request?.Invoke(serviceId, request_gdp);
                if (response_gdp != null)
                {
                    return response_gdp.Handle;
                }
                return IntPtr.Zero;
            };
            m_callback = req2;
            NativeMethods.gravity_set_serviceprovider_callback(Handle, req2);
        }
    }
}
