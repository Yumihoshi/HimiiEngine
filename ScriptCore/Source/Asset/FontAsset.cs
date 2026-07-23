using System;
using System.Runtime.InteropServices;

namespace HimiiEngine
{
    public class FontAsset
    {
        public ulong Handle { get; private set; }

        public FontAsset(ulong handle)
        {
            Handle = handle;
        }

        public static FontAsset? GetDefault()
        {
            if (InternalCalls.FontAsset_GetDefaultHandle == null)
                return null;
            ulong handle = InternalCalls.FontAsset_GetDefaultHandle();
            return handle == 0 ? null : new FontAsset(handle);
        }

        public void PreloadCharacters(string text)
        {
            if (InternalCalls.FontAsset_PreloadCharacters == null)
                return;
            IntPtr textPointer = Marshal.StringToCoTaskMemUTF8(text ?? string.Empty);
            InternalCalls.FontAsset_PreloadCharacters(Handle, textPointer);
            Marshal.FreeCoTaskMem(textPointer);
        }

        public void PreloadTextAsync(string text)
        {
            if (InternalCalls.FontAsset_PreloadTextAsync == null)
                return;
            IntPtr textPointer = Marshal.StringToCoTaskMemUTF8(text ?? string.Empty);
            InternalCalls.FontAsset_PreloadTextAsync(Handle, textPointer);
            Marshal.FreeCoTaskMem(textPointer);
        }

        public void WaitForPendingGenerations()
        {
            InternalCalls.FontAsset_WaitForPendingGenerations?.Invoke(Handle);
        }
    }
}
