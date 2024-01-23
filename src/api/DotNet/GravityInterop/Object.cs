using System;
using System.Collections;
using System.Collections.Generic;

namespace GravityInterop.Base
{

    public abstract class Object : IDisposable
    {
        internal readonly DeleterHandle m_instance;
        private bool disposedValue;

        protected Object(IntPtr ptr, Deleter deleter)
        {
            if (ptr == IntPtr.Zero)
            {
                throw new ArgumentNullException(nameof(ptr));
            }

            m_instance = new DeleterHandle(ptr, deleter);
        }

        public IntPtr Handle
        {
            get
            {
                if (m_instance.IsInvalid)
                {
                    throw new ObjectDisposedException(GetType().Name);
                }

                return m_instance.Handle;
            }
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects)
                }

                // TODO: free unmanaged resources (unmanaged objects) and override finalizer
                // TODO: set large fields to null
                m_instance.Dispose();
                disposedValue = true;
            }
        }

        // TODO: override finalizer only if 'Dispose(bool disposing)' has code to free unmanaged resources
        ~Object()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

}
