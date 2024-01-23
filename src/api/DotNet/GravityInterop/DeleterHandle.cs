using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;

namespace GravityInterop.Base
{
    [SuppressUnmanagedCodeSecurity]
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void Deleter(IntPtr ptr);

    public class DeleterHandle : IDisposable
    {
        private IntPtr handle;
        private Deleter deleter;

        public IntPtr Handle => handle;

        public bool IsInvalid => handle == IntPtr.Zero;

        public DeleterHandle(IntPtr ptr, Deleter deleter)
        {
            handle = ptr;
            this.deleter = deleter;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        public void Dispose(bool disposing)
        {
            if (handle == IntPtr.Zero)
            {
                return;
            }

            deleter?.Invoke(handle);
            handle = IntPtr.Zero;
        }

        ~DeleterHandle()
        {
            Dispose(false);
        }

    }
}
