namespace HimiiEngine
{
    public static class Time
    {
        public static float DeltaTime
        {
            get
            {
                if (InternalCalls.Time_GetDeltaTime == null)
                    return 0.0f;
                return InternalCalls.Time_GetDeltaTime();
            }
        }
    }
}
