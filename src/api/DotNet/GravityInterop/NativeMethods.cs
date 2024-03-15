using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace GravityInterop
{
    internal static class NativeMethods 
    {

        private const string dllName = "GravityWrapper";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void subscription_filled(IntPtr dpArray, int length);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void request_filled([MarshalAs(UnmanagedType.LPStr)] string serviceID, [MarshalAs(UnmanagedType.LPStr)] string requestID, IntPtr responseDP);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr service_request([MarshalAs(UnmanagedType.LPStr)] string serviceID, IntPtr gravityDp);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ondebug_cb(IntPtr message, int size);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_create_node([MarshalAs(UnmanagedType.LPStr)] string componentId);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_delete_node(IntPtr node);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_register_serviceprovider(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string serviceId, GravityTransportTypes.Types type, IntPtr sp);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_create_requestor();

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_set_requestor_callback(IntPtr gi_req, request_filled cb);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_delete_requestor(IntPtr requestor);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gravity_request_async(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string servName, IntPtr requestdp, IntPtr requestor, [MarshalAs(UnmanagedType.LPStr)] string requestId);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_request(IntPtr node, [MarshalAs(UnmanagedType.LPStr)] string servName, IntPtr requestDp, int timeoutMs);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_create_serviceprovider();

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_set_serviceprovider_callback(IntPtr gi_sp, service_request cb);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_delete_serviceprovider(IntPtr serviceprovider);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_create_dataproduct([MarshalAs(UnmanagedType.LPStr)] string dpId);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_delete_dataproduct(IntPtr dp);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gravity_getsize_dataproduct(IntPtr dp);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_getptr_dataproduct(IntPtr dp, IntPtr data, int size);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_setdata_dataproduct(IntPtr dp, IntPtr data, int size);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr gravity_create_subscriber();

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_delete_subscriber(IntPtr subscriber);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_set_subscriber_callback(IntPtr subscriber, [MarshalAs(UnmanagedType.FunctionPtr)] subscription_filled cb);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_node_publish(IntPtr gn, IntPtr dp);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_node_subscribe([MarshalAs(UnmanagedType.LPStr)] string dataProductId, IntPtr gn, IntPtr subscriber);

        [DllImport(dllName, CallingConvention =CallingConvention.Cdecl)]
        internal static extern void gravity_register_debug_callback(ondebug_cb cb);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void gravity_register_dataproduct(IntPtr gn, [MarshalAs(UnmanagedType.LPStr)] string dpId, GravityTransportTypes.Types type);


    }
}
