using System.Runtime.InteropServices;

namespace Himii
{
    public static class SceneManager
    {
        /// <summary>
        /// 加载场景文件（相对于项目 assets 的路径，例如 scenes/Level1.himii）。
        /// 仅在运行时（Play）有效。
        /// </summary>
        public static void LoadScene(string scenePath)
        {
            if (string.IsNullOrEmpty(scenePath) || InternalCalls.SceneManager_LoadScene == null)
                return;

            InternalCalls.SceneManager_LoadScene(Marshal.StringToCoTaskMemUTF8(scenePath));
        }
    }
}
