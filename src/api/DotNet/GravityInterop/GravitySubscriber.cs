using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using static GravityInterop.NativeMethods;

namespace GravityInterop
{

    public class GravitySubscriber : Base.Object
    {
        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private subscription_filled m_callback;

        internal static IntPtr Create()
        {
            return NativeMethods.gravity_create_subscriber();
        }

        public GravitySubscriber(GravityNode.SubscriptionFilled cb) : base(Create(), NativeMethods.gravity_delete_subscriber)
        {
            subscription_filled cb2 = (dpArray, len) =>
            {
                List<GravityDataProduct> newDps = new List<GravityDataProduct>();
                IntPtr[] newDPHandles = new IntPtr[len];
                Marshal.Copy(dpArray, newDPHandles, 0, len);

                for (int i = 0; i < len; i++)
                {
                    newDps.Add(new GravityDataProduct(newDPHandles[i]));
                }
                cb?.Invoke(newDps);
            };
            m_callback = cb2;
            NativeMethods.gravity_set_subscriber_callback(Handle, cb2);
        }
    }
}
