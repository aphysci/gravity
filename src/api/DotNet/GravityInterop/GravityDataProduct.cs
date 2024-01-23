using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace GravityInterop
{

    public class GravityDataProduct : Base.Object
    {
        internal static IntPtr Create(string dpId)
        {
            return NativeMethods.gravity_create_dataproduct(dpId);
        }

        public GravityDataProduct(string dpId) : base(Create(dpId), NativeMethods.gravity_delete_node)
        {
        }

        public GravityDataProduct(IntPtr handle) : base(handle, NativeMethods.gravity_delete_node)
        {
        }

        public byte[] getBytes()
        {
            int sz = NativeMethods.gravity_getsize_dataproduct(Handle);
            IntPtr nativeMem = Marshal.AllocHGlobal(sz);
            NativeMethods.gravity_getptr_dataproduct(Handle, nativeMem, sz);
            byte[] toReturn = new byte[sz];
            Marshal.Copy(nativeMem, toReturn, 0, sz);
            Marshal.FreeHGlobal(nativeMem);
            return toReturn;
        }

        public void setFromBytes(byte[] bytes)
        {
            IntPtr nativeMem = Marshal.AllocHGlobal(bytes.Length);
            Marshal.Copy(bytes, 0, nativeMem, bytes.Length);
            NativeMethods.gravity_setdata_dataproduct(Handle, nativeMem, bytes.Length);
            Marshal.FreeHGlobal(nativeMem);
        }
    }
}
