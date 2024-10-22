#include "hmc_winapi_util.h"

namespace HMC = hmc_winapi_util;

// if
HMC::User32::operator bool() const
{
    return m_hModule != nullptr;
}

// !
bool HMC::User32::operator!() const
{
    return m_hModule == nullptr;
}

HMC::User32::User32()
{
    m_hModule = ::LoadLibraryW(L"user32.dll");
}

HMC::User32::~User32()
{
    if (m_hModule != nullptr)
    {
        ::FreeLibrary(m_hModule);
    }
}

HMC::Shell32::Shell32()
{
    m_hModule = ::LoadLibraryW(L"shell32.dll");
}

HMC::Shell32::~Shell32()
{
    if (m_hModule != nullptr)
    {
        ::FreeLibrary(m_hModule);
    }
}

BOOL HMC::User32::UpdateWindow(_In_ HWND hWnd)
{
    if (!m_hModule)
    {
        return ::UpdateWindow(hWnd);
    }

    PFN_UpdateWindow pUpdateWindow = (PFN_UpdateWindow)::GetProcAddress(m_hModule, "UpdateWindow");

    if (!pUpdateWindow)
    {
        return 0;
    }

    return pUpdateWindow(hWnd);
}

LRESULT HMC::User32::SendMessageW(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    if (!m_hModule)
    {
        return ::SendMessageW(hWnd, Msg, wParam, lParam);
    }

    PFN_SendMessageW pSendMessageW = (PFN_SendMessageW)::GetProcAddress(m_hModule, "SendMessageW");

    if (!pSendMessageW)
    {
        return 0;
    }

    return pSendMessageW(hWnd, Msg, wParam, lParam);
}

HICON HMC::Shell32::ExtractIconW(_Reserved_ HINSTANCE hInst, _In_ LPCWSTR pszExeFileName, UINT nIconIndex)
{
    if (!m_hModule)
    {
        return ::ExtractIconW(hInst, pszExeFileName, nIconIndex);
    }

    PFN_ExtractIconW pExtractIconW = (PFN_ExtractIconW)::GetProcAddress(m_hModule, "ExtractIconW");

    if (!pExtractIconW)
    {
        return 0;
    }

    return pExtractIconW(hInst, pszExeFileName, nIconIndex);
}

int HMC::User32::GetWindowTextW(_In_ HWND hWnd, _Out_writes_(nMaxCount) LPWSTR lpString, _In_ int nMaxCount)
{

    if (!m_hModule)
    {
        return ::GetWindowTextW(hWnd, lpString, nMaxCount);
    }

    PFN_GetWindowTextW pGetWindowTextW = (PFN_GetWindowTextW)::GetProcAddress(m_hModule, "GetWindowTextW");

    if (!pGetWindowTextW)
    {
        return 0;
    }

    return pGetWindowTextW(hWnd, lpString, nMaxCount);
}

HWND HMC::User32::GetDlgItem(_In_opt_ HWND hDlg, _In_ int nIDDlgItem)
{

    if (!m_hModule)
    {
        return ::GetDlgItem(hDlg, nIDDlgItem);
    }

    PFN_GetDlgItem pGetDlgItem = (PFN_GetDlgItem)::GetProcAddress(m_hModule, "GetDlgItem");

    if (!pGetDlgItem)
    {
        return (HWND)0;
    }

    return pGetDlgItem(hDlg, nIDDlgItem);
}

int HMC::User32::GetWindowTextLengthW(_In_ HWND hWnd)
{

    if (!m_hModule)
    {
        return ::GetWindowTextLengthW(hWnd);
    }

    PFN_GetWindowTextLengthW pGetWindowTextLengthW = (PFN_GetWindowTextLengthW)::GetProcAddress(m_hModule, "GetWindowTextLengthW");

    if (!pGetWindowTextLengthW)
    {
        return 0;
    }

    return pGetWindowTextLengthW(hWnd);
}

int HMC::User32::InternalGetWindowText(_In_ HWND hWnd, _Out_writes_to_(cchMaxCount, return +1) LPWSTR pString, _In_ int cchMaxCount)
{

    if (!m_hModule)
    {
        return ::GetWindowTextLengthW(hWnd);
    }

    PFN_GetWindowTextLengthW pGetWindowTextLengthW = (PFN_GetWindowTextLengthW)::GetProcAddress(m_hModule, "GetWindowTextLengthW");

    if (!pGetWindowTextLengthW)
    {
        return 0;
    }

    return pGetWindowTextLengthW(hWnd);
}

BOOL HMC::User32::SetWindowTextW(_In_ HWND hWnd, _In_opt_ LPCWSTR lpString)
{

    if (!m_hModule)
    {
        return ::SetWindowTextW(hWnd, lpString);
    }

    PFN_SetWindowTextW pSetWindowTextW = (PFN_SetWindowTextW)::GetProcAddress(m_hModule, "SetWindowTextW");

    if (!pSetWindowTextW)
    {
        return 0;
    }

    return pSetWindowTextW(hWnd, lpString);
}