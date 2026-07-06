using System;
using System.Runtime.InteropServices;

namespace HimiiEngine
{
    public static class SceneManager
    {
        /// <summary>
        /// 加载场景文件（相对于项目 assets 的路径，例如 scenes/Level1.himii）。
        /// 仅在运行时（Play）有效。
        /// </summary>
        /// <returns>加载成功返回 true。</returns>
        public static bool LoadScene(string scenePath)
        {
            if (string.IsNullOrEmpty(scenePath) || InternalCalls.SceneManager_LoadScene == null)
                return false;

            IntPtr scenePathPointer = Marshal.StringToCoTaskMemUTF8(scenePath);
            try
            {
                return InternalCalls.SceneManager_LoadScene(scenePathPointer) != 0;
            }
            finally
            {
                Marshal.FreeCoTaskMem(scenePathPointer);
            }
        }

        /// <summary>
        /// 当前活动场景的 assets 相对路径（例如 scenes/Level1.himii）。未加载时返回空字符串。
        /// </summary>
        public static string ActiveScenePath
        {
            get
            {
                if (InternalCalls.SceneManager_GetActiveScenePath == null)
                    return string.Empty;

                IntPtr pathPointer = InternalCalls.SceneManager_GetActiveScenePath();
                return pathPointer == IntPtr.Zero
                    ? string.Empty
                    : Marshal.PtrToStringUTF8(pathPointer) ?? string.Empty;
            }
        }
    }
}
