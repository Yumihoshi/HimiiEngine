using System;
using System.Runtime.InteropServices;

namespace HimiiEngine
{
    public enum LogLevel
    {
        Trace = 0,
        Info = 1,
        Warning = 2,
        Error = 3
    }

    public static class Log
    {
        public static void Info(string message) => Write(LogLevel.Info, message);
        public static void Warning(string message) => Write(LogLevel.Warning, message);
        public static void Error(string message) => Write(LogLevel.Error, message);

        private static void Write(LogLevel level, string message)
        {
            if (InternalCalls.NativeLog == null)
                return;

            message ??= string.Empty;
            IntPtr ptr = Marshal.StringToCoTaskMemUTF8(message);
            try
            {
                InternalCalls.NativeLog((int)level, ptr);
            }
            finally
            {
                Marshal.FreeCoTaskMem(ptr);
            }
        }
    }
}
