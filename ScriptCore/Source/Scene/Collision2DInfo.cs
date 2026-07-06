using System.Runtime.InteropServices;

namespace HimiiEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Collision2DInfo
    {
        public ulong OtherEntityID;
        public Vector2 Normal;
        public Vector2 Point;
    }
}
