namespace HimiiEngine
{
    /// <summary>音频资产句柄包装（与 C++ SoundAsset / AssetHandle 对应）。</summary>
    public readonly struct SoundAsset
    {
        public ulong Handle { get; }

        public SoundAsset(ulong handle)
        {
            Handle = handle;
        }

        public bool IsValid => Handle != 0UL;
    }
}
