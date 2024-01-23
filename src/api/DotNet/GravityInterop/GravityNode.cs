using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using static GravityInterop.NativeMethods;

namespace GravityInterop
{
    namespace GravityTransportTypes
    {
        /**
         * Network transport protocols available on a Gravity system. 
         */
        public enum Types
        {
            TCP = 0, ///< Transmission Control Protocol
            INPROC = 1, ///< In-process (Inter-thread) Communication 
            PGM = 2, ///< Pragmatic General Multicast Protocol
            EPGM = 3, ///< Encapsulated PGM
            IPC = 4 ///< Inter-Process Communication
        };
    }


    public class GravityNode : Base.Object
    {
        public delegate void SubscriptionFilled(List<GravityDataProduct> dps);
        public delegate GravityDataProduct Request(string serviceId,  GravityDataProduct dp);
        public delegate void RequestFilled(string serviceId, string requestId,  GravityDataProduct dp);

        internal static IntPtr Create(string componentId)
        {
            return NativeMethods.gravity_create_node(componentId);
        }

        public GravityNode(string componentId) : base(Create(componentId), NativeMethods.gravity_delete_node)
        {

        }

        public void Subscribe(string dataProductId, GravitySubscriber subscriber)
        {
            NativeMethods.gravity_node_subscribe(dataProductId, this.Handle, subscriber.Handle);
        }

        public void Publish(GravityDataProduct dp)
        {
            NativeMethods.gravity_node_publish(this.Handle, dp.Handle);
        }

        public void RegisterDataProduct(string dataProductId, GravityTransportTypes.Types type) 
        {
            if (type == GravityTransportTypes.Types.IPC && RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                throw new InvalidOperationException("IPC transport is not available on Windows");
            NativeMethods.gravity_register_dataproduct(this.Handle, dataProductId, type);
        }

        public void RegisterServiceProvider(string serviceId, GravityTransportTypes.Types type, GravityServiceProvider provider)
        {
            if (type == GravityTransportTypes.Types.IPC && RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                throw new InvalidOperationException("IPC transport is not available on Windows");
            NativeMethods.gravity_register_serviceprovider(this.Handle, serviceId, type, provider.Handle);
        }

        public void ServiceRequestAsync(GravityServiceRequestor req, string serviceId, string reqId, GravityDataProduct reqDP)
        {
            NativeMethods.gravity_request_async(Handle, serviceId, reqDP.Handle, req.Handle, reqId);
        }
    }
}
