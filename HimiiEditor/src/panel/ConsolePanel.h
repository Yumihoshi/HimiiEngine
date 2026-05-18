#pragma once

namespace Himii
{
    class ConsolePanel
    {
    public:
        void OnImGuiRender(bool* pOpen);

    private:
        bool m_ShowEngineLogs = false;
        bool m_AutoScroll = true;
    };
}
