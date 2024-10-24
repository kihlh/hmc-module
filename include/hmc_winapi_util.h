#pragma once

// 防止重复导入
#ifndef MODE_INTERNAL_INCLUDE_HMC_WINAPI_UTIL_HPP
#define MODE_INTERNAL_INCLUDE_HMC_WINAPI_UTIL_HPP

#include <windows.h>

namespace hmc_winapi_util
{
    class User32
    {
    private:
        HINSTANCE m_hModule;

    public:
        User32();
        ~User32();

        typedef LRESULT(WINAPI *PFN_SendMessageW)(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);
        typedef BOOL (WINAPI *PFN_UpdateWindow)(_In_ HWND);

        typedef int(WINAPI *PFN_GetWindowTextW)(_In_ HWND hWnd, LPWSTR lpString, _In_ int nMaxCount);
        typedef HWND(WINAPI *PFN_GetDlgItem)(_In_opt_ HWND hDlg, _In_ int nIDDlgItem);
        typedef int(WINAPI *PFN_GetWindowTextLengthW)(_In_ HWND hWnd);
        typedef int(WINAPI *PFN_InternalGetWindowText)(_In_ HWND hWnd,LPWSTR pString, _In_ int cchMaxCount);
        typedef BOOL(WINAPI *PFN_SetWindowTextW)(_In_ HWND hWnd, _In_opt_ LPCWSTR lpString);

        BOOL UpdateWindow(_In_ HWND hWnd);
        LRESULT SendMessageW(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
        BOOL SetWindowTextW(_In_ HWND hWnd, _In_opt_ LPCWSTR lpString);

        int GetWindowTextW(_In_ HWND hWnd, LPWSTR lpString, _In_ int nMaxCount) const;
        HWND GetDlgItem(_In_opt_ HWND hDlg, _In_ int nIDDlgItem);
        int GetWindowTextLengthW(_In_ HWND hWnd);
        int InternalGetWindowText(_In_ HWND hWnd, LPWSTR pString, _In_ int cchMaxCount) const;

        // if
        operator bool() const;
        // !
        bool operator!() const;
    };

    class Shell32
    {
    private:
        HINSTANCE m_hModule;

    public:
        Shell32();
        ~Shell32();

        typedef HICON(WINAPI *PFN_ExtractIconW)(HINSTANCE hInst, LPCWSTR pszExeFileName, UINT nIconIndex);

        HICON ExtractIconW(_Reserved_ HINSTANCE hInst, _In_ LPCWSTR pszExeFileName, UINT nIconIndex);
    };
}

#endif // MODE_INTERNAL_INCLUDE_HMC_WINAPI_UTIL_HPP