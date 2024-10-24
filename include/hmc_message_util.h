#pragma once

// 防止重复导入
#ifndef MODE_INTERNAL_INCLUDE_HMC_MESSAGE_UTIL_HPP
#define MODE_INTERNAL_INCLUDE_HMC_MESSAGE_UTIL_HPP

#include <windows.h>
#include <string>

namespace hmc_message_util
{
    class hmc_Window_Util
    {
    public:
        HWND m_hWnd;
        std::wstring title;
        std::wstring className;
    private:

    };

    class Message : protected hmc_Window_Util
    {
    public:
    private:
    };

    class MessageBox : protected hmc_Window_Util
    {
    public:
    private:
    };

    class Notification : protected hmc_Window_Util
    {
    public:
    private:
    };
}
#endif // MODE_INTERNAL_INCLUDE_HMC_MESSAGE_UTIL_HPP