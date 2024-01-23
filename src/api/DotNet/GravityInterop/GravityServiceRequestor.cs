using GravityInterop;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using static GravityInterop.NativeMethods;

namespace GravityInterop
{
    public class GravityServiceRequestor : GravityInterop.Base.Object
    {
        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private request_filled m_callback;

        internal static IntPtr Create()
        {
            return NativeMethods.gravity_create_requestor();
        }

        public GravityServiceRequestor(GravityNode.RequestFilled requestFilled) : base(Create(), NativeMethods.gravity_delete_requestor)
        {
            request_filled rf2 = (serviceId, requestId, dpPtr) =>
            {
                requestFilled?.Invoke(serviceId, requestId, new GravityDataProduct(dpPtr));
            };
            m_callback = rf2;
            NativeMethods.gravity_set_requestor_callback(Handle, rf2);
        }
    }
}
