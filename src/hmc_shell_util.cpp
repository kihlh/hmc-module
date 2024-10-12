#include "hmc_shell_util.h"
#include "hmc_windows_util.h"
#include "hmc_string_util.h"
#include "hmc_util.h"
#include "hmc_registr_util.h"
#include "hmc_string_util.h"

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <thread>

#include <ShlObj.h>

#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include <fstream>

#include <olectl.h>
#include <ShObjIdl.h> //For IShellItemImageFactory
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "Gdiplus")

#ifndef _GDIPLUS_H

#include <gdiplus.h>
#endif // !_GDIPLUS_HPP

namespace HS = hmc_string_util;

class GetThumbnail
{
public:
    /**
     * @brief 转码
     *
     * @param format
     * @param pClsid
     * @return
     */
    static int GetEncoderClsid(const WCHAR *format, CLSID *pClsid);
    /**
     * @brief 转码到二进制
     *
     * @param image
     * @param formatMimeType
     * @param result
     */
    static void WriteToImageByte(Gdiplus::Bitmap *image, LPCWSTR formatMimeType, std::vector<BYTE> &result);
    /**
     * @brief 取出为指定格式
     *
     * @param hbitmap
     * @param result
     * @param format
     * @param formatMimeType
     */
    static void GetBitmapPixel(HBITMAP hbitmap, std::vector<BYTE> &result, Gdiplus::PixelFormat format = PixelFormat32bppARGB, LPCWSTR formatMimeType = L"image/png");
};

// 预赋值头
namespace hmc_shell_util
{

    typedef struct ch_copy_REPARSE_DATA_BUFFER
    {
        ULONG ReparseTag;
        USHORT ReparseDataLength;
        USHORT Reserved;
        union
        {
            struct
            {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                ULONG Flags;
                WCHAR PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct
            {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                WCHAR PathBuffer[1];
            } MountPointReparseBuffer;
            struct
            {
                UCHAR DataBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
            } GenericReparseBuffer;
        };
    } ch_copy_REPARSE_DATA_BUFFER, *ch_copy_PREPARSE_DATA_BUFFER;

    // bool SelectFolderV1(const std::wstring &Title, const std::wstring &SelectFolderPath);
    // bool SelectFolder(std::wstring &folderPath);
    // bool SelectFolders(std::vector<std::wstring> &folderPaths);
    // bool SelectFiles(std::vector<std::wstring> &filePaths);
    // bool SelectFile(std::wstring &FilePath);
    // bool SelectFile(std::wstring &FilePath, std::map<std::wstring, std::wstring> filtr);
    // bool SelectFiles(std::vector<std::wstring> &FilePaths, const std::map<std::wstring, std::wstring> &filtr);
}

std::vector<hmc_shell_util::GetTaryIconList::TaryIcon> hmc_shell_util::GetTaryIconList::getTrayList()
{
    std::vector<TaryIcon> result = {};

    // win11
    if (::FindWindowW(L"TopLevelWindowForOverflowXamlIsland", NULL))
    {
        return result;
    }

    std::vector<HWND> hWndList = {GetSystemTrayHwnd(), GetNotifyOverflowTryHwnd()};

    for (size_t i = 0; i < hWndList.size(); i++)
    {
        HWND hwnd = hWndList[i];

        if (hwnd == NULL)
            continue;

        auto list = EnumCommctrlList(hwnd);

        for (size_t i = 0; i < list.size(); i++)
        {
            result.push_back(list[i]);
        }
    }

    return result;
}

std::wstring hmc_shell_util::GetTaryIconList::getTrayListJsonW()
{
    std::wstring result = L"[";
    result.reserve(2);

    auto data_list = getTrayList();

    if (data_list.empty())
    {
        result.push_back(']');
        return result;
    }

    result.reserve(4096);

    for (size_t i = 0; i < data_list.size(); i++)
    {
        auto data = data_list[i];
        std::wstring obj = LR"({ "info":"{info}","path":"{path}","handle":{handle} })";
        obj.reserve(54 + MAX_PATH * 2);

        obj.replace(obj.find(L"{info}"), sizeof("{info}") - 1, hmc_string_util::escapeJsonString(data.title));
        obj.replace(obj.find(L"{path}"), sizeof("{path}") - 1, hmc_string_util::escapeJsonString(data.path));
        obj.replace(obj.find(L"{handle}"), sizeof("{handle}") - 1, std::to_wstring((long long)data.hwnd));

        if (i + 1 < result.size())
        {
            result.push_back(',');
        }

        result.append(obj);
    }

    result.push_back(']');

    return result;
}

BOOL hmc_shell_util::GetTaryIconList::IsSystemFor64bit()
{
    SYSTEM_INFO SystemInfo;
    ::GetNativeSystemInfo(&SystemInfo);
    if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return TRUE;
    else
        return FALSE;
}

HWND hmc_shell_util::GetTaryIconList::GetNotifyOverflowTryHwnd()
{
    HWND hWnd = ::FindWindowW(L"NotifyIconOverflowWindow", NULL);

    if (hWnd != NULL)
    {
        hWnd = ::FindWindowExW(hWnd, NULL, L"ToolbarWindow32", NULL);
    }

    return hWnd;
}

HWND hmc_shell_util::GetTaryIconList::GetSystemTrayHwnd()
{
    HWND hWnd = ::FindWindowW(L"Shell_TrayWnd", NULL);
    if (hWnd)
    {
        hWnd = ::FindWindowExW(hWnd, NULL, L"TrayNotifyWnd", NULL);
        if (hWnd)
        {
            hWnd = ::FindWindowExW(hWnd, NULL, L"SysPager", NULL);
            if (hWnd)
                hWnd = ::FindWindowExW(hWnd, NULL, L"ToolbarWindow32", NULL);
        }
    }
    if (!hWnd)
        return NULL;
    return hWnd;
};

std::vector<hmc_shell_util::GetTaryIconList::TaryIcon> hmc_shell_util::GetTaryIconList::EnumCommctrlList(HWND hWnd)
{
    std::vector<TaryIcon> result = {};
    if (hWnd == NULL)
        return result;

    // 获取托盘进程ID
    DWORD dwProcessId = 0;
    ::GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (dwProcessId == 0)
        return result;

    // 获取托盘进程句柄
    HANDLE hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (hProcess == NULL)
        return result;

    // 分配内存，用来接收 TBBUTTON 结构体
    LPVOID Pointer_Icon_Buttons = ::VirtualAllocEx(hProcess, 0, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    // gc
    std::shared_ptr<void> _shared_free_handle(nullptr, [&](void *)
                                              {
            if (Pointer_Icon_Buttons != NULL) {
                ::VirtualFreeEx(hProcess, Pointer_Icon_Buttons, 0, MEM_RELEASE);
            }
            if (hProcess != NULL) {
                ::CloseHandle(hProcess);
            } });

    if (Pointer_Icon_Buttons == NULL)
        return result;

    // 初始化
    BYTE Byte_Buffer_Pointer[1024] = {0};
    uintptr_t Index_Icon_Buttons = 0;
    HWND Main_HWND = NULL;

    long Index_Icon_Buttons_id = 0;
    int Index_data_Close_Set = 12;
    int Index_String_Close_Set = 18;

    // 判断 x64(64位和32位字数不一样)
    if (IsSystemFor64bit())
    {
        Index_data_Close_Set = Index_data_Close_Set + 4;
        Index_String_Close_Set = Index_String_Close_Set + 6;
    }

    // 获取托盘图标个数
    size_t Tray_Length = 0;
    Tray_Length = ::SendMessage(hWnd, _TB_BUTTONCOUNT, 0, 0);

    if (Tray_Length == 0)
        return result;

    // 遍历托盘
    for (size_t i = 0; i < Tray_Length; i++)
    {
        // 获取 TBBUTTON 结构
        if (!::SendMessage(hWnd, _TB_GETBUTTON, i, (LPARAM)Pointer_Icon_Buttons))
            break;

        void *pointer = (LPVOID)((uintptr_t)Pointer_Icon_Buttons + Index_data_Close_Set);

        // 获取 TBBUTTON.dwData
        if (!::ReadProcessMemory(hProcess, pointer, &Index_Icon_Buttons, 4, 0))
            break;

        // 解析文本
        if (Index_Icon_Buttons)
        {
            if (!::ReadProcessMemory(hProcess, (LPCVOID)Index_Icon_Buttons, Byte_Buffer_Pointer, 1024, 0))
                break;

            HWND icon_hwnd = (*((HWND *)Byte_Buffer_Pointer));

            if (icon_hwnd == NULL)
                continue;

            std::wstring path = (WCHAR *)Byte_Buffer_Pointer + Index_String_Close_Set;
            std::wstring title = (WCHAR *)Byte_Buffer_Pointer + Index_String_Close_Set + MAX_PATH;
            long index = Index_Icon_Buttons_id;

            result.push_back(TaryIcon(icon_hwnd, path, title, index));
        }

        Index_Icon_Buttons = 0;
    }

    return result;
}

// ? API  function

bool hmc_shell_util::isSystemFor64bit()
{
    SYSTEM_INFO SystemInfo;
    ::GetNativeSystemInfo(&SystemInfo);
    if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return TRUE;
    else
        return FALSE;
}

int hmc_shell_util::trashFile(std::wstring FromPath, bool bRecycle, bool isShow)
{
    SHFILEOPSTRUCTW FileOp = {0};
    if (bRecycle)
    {
        if (isShow)
        {
            FileOp.fFlags |= FOF_ALLOWUNDO;
        }
        else
        {
            FileOp.fFlags |= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
        }
    }
    FileOp.pFrom = FromPath.c_str();
    FileOp.pTo = NULL;        // 一定要是NULL
    FileOp.wFunc = FO_DELETE; // 删除操作
    return ::SHFileOperationW(&FileOp);
}

namespace hmc_shell_util
{

    bool hmc_shell_util::clearTrash(HWND hwnd, std::wstring RootPath, DWORD dwFlags)
    {
        return S_OK == ::SHEmptyRecycleBinW(hwnd, RootPath.empty() ? NULL : RootPath.c_str(), dwFlags);
    }

    bool setConversionStatus(HWND hwnd = NULL, DWORD sentencePatterns = IME_CMODE_ALPHANUMERIC, DWORD conversionModeValue = IME_SMODE_AUTOMATIC)
    {
        auto is_ok = false;

        // 获取输入法上下文
        HIMC hImc = ImmGetContext(hwnd);

        if (hImc == nullptr)
        {
            return false;
        }

        is_ok = ::ImmSetConversionStatus(hImc, sentencePatterns, conversionModeValue) != 0;

        if (is_ok)
        {
            is_ok = ::ImmReleaseContext(hwnd, hImc) != 0;
        }

        return is_ok;
    }

    bool hmc_shell_util::setShortcutLink(std::wstring lnkPath, std::wstring FilePath, std::wstring work_dir = L"", std::wstring desc = L"", std::wstring args = L"", DWORD iShowCmd = 0, std::wstring icon = L"", DWORD iconIndex = 0)
    {

        bool result = false;

        HRESULT hRes = ::CoInitialize(NULL);
        if (!SUCCEEDED(hRes))
        {
            return result;
        }

        IShellLinkW *pIShellLink;
        hRes = ::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pIShellLink);
        if (!SUCCEEDED(hRes))
        {
            return result;
        }
        hRes = pIShellLink->SetPath(FilePath.c_str());
        if (!SUCCEEDED(hRes))
        {
            return result;
        }
        hRes = pIShellLink->SetDescription(desc.c_str());
        if (!SUCCEEDED(hRes))
        {
            return result;
        }
        hRes = pIShellLink->SetWorkingDirectory(work_dir.c_str());
        if (!SUCCEEDED(hRes))
        {
            return result;
        }
        hRes = pIShellLink->SetArguments(args.c_str());
        if (!SUCCEEDED(hRes))
        {
            return result;
        }

        hRes = pIShellLink->SetShowCmd(iShowCmd);
        if (!SUCCEEDED(hRes))
        {
            return result;
        }

        hRes = pIShellLink->SetIconLocation(icon.c_str(), iconIndex);
        if (!SUCCEEDED(hRes))
        {
            return result;
        }
        IPersistFile *pIPersistFile;
        hRes = pIShellLink->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
        if (!SUCCEEDED(hRes))
        {
            return result;
        }

        hRes = pIPersistFile->Save(lnkPath.c_str(), FALSE);
        if (!SUCCEEDED(hRes))
        {

            return result;
        }

        pIPersistFile->Release();
        pIShellLink->Release();

        result = true;

        return result;
    }

}

int hmc_shell_util::ShellOpen::openInExplorer(std::wstring Path)
{

    HINSTANCE hResult = ShellExecuteW(NULL, L"open", L"explorer.exe", Path.c_str(), NULL, SW_SHOWNORMAL);

    return (int)(UINT_PTR)hResult;
}

int hmc_shell_util::ShellOpen::showItemInFolder(std::wstring Path, bool isSelect)
{
    std::wstring explor_cmd = L"/select, ";

    explor_cmd.reserve(Path.size() + 15);

    if (!isSelect)
    {
        explor_cmd.clear();
    }

    explor_cmd.push_back('"');
    explor_cmd.append(Path.c_str());
    explor_cmd.push_back('"');

    HINSTANCE hResult = ShellExecuteW(NULL, L"open", L"explorer.exe", explor_cmd.c_str(), NULL, SW_SHOWNORMAL);

    return (int)(UINT_PTR)hResult;
}

int hmc_shell_util::ShellOpen::openUrl(std::wstring Url, bool isCurrentBrowser)
{
    int result = 0;

    // if (!isCurrentBrowser)
    // {
    HINSTANCE hResult = ShellExecuteW(NULL, L"open", Url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    result = (int)(UINT_PTR)hResult;
    // }

    return result;
}

int hmc_shell_util::ShellOpen::openApp(std::wstring AppPath, std::wstring Command, std::wstring cwd, bool isHide, bool isAdmin)
{
    const LPCWSTR flag = isAdmin ? L"runas" : L"open";
    const int sw_command = isHide ? 0 : 5;

    HINSTANCE hResult = ::ShellExecuteW(
        // hwnd
        NULL,
        // lpOperation
        flag,
        // lpFile
        AppPath.c_str(),
        // lpParameters
        Command.empty() ? NULL : Command.c_str(),
        // lpDirectory
        cwd.empty() ? NULL : cwd.c_str(),
        // nShowCmd
        sw_command);

    return (int)(UINT_PTR)hResult;
}

bool hmc_shell_util::chShortcutLinkItem::isValid()
{
    return !path.empty();
}

hmc_shell_util::chShortcutLinkItem::chShortcutLinkItem()
{
    // 目标路径
    path = std::wstring();
    // 启动属性
    showCmd = 0;
    // 启动命令
    args = std::wstring();
    // 描述
    desc = std::wstring();
    // 进程图标(例如来自1.dll)
    icon = std::wstring();
    // 图标索引位置
    iconIndex = 0;
    // 工作环境文件夹
    cwd = std::wstring();
    // hotkey
    hotkey = 0;
}

std::wstring hmc_shell_util::chShortcutLinkItem::to_json()
{
    std::wstring result;
    size_t size_len = path.size() + args.size() + desc.size() + icon.size() + cwd.size() + MAX_PATH;
    result.reserve(size_len);
    result.push_back('{');

    const wchar_t* format = 
    LR"( "path"      : {path}      ,)"
    LR"( "showCmd"   : {showCmd}   ,)"
    LR"( "args"      : {args}      ,)"
    LR"( "desc"      : {desc}      ,)"
    LR"( "icon"      : {icon}      ,)"
    LR"( "iconIndex" : {iconIndex} ,)"
    LR"( "cwd"       : {cwd}       ,)"
    LR"( "hotkey"    : {hotkey}     )"
    ;

    result.append(format);

    hmc_string_util::replace(result, L"{path}", hmc_string_util::escapeJsonString(path,true));

    hmc_string_util::replace(result, L"{args}", hmc_string_util::escapeJsonString(args,true));
    hmc_string_util::replace(result, L"{desc}", hmc_string_util::escapeJsonString(desc,true));
    hmc_string_util::replace(result, L"{icon}", hmc_string_util::escapeJsonString(icon,true));
    hmc_string_util::replace(result, L"{cwd}", hmc_string_util::escapeJsonString(cwd,true));
    hmc_string_util::replace(result, L"{showCmd}", std::to_wstring(showCmd));
    hmc_string_util::replace(result, L"{iconIndex}", std::to_wstring(iconIndex));
    hmc_string_util::replace(result, L"{hotkey}", std::to_wstring(hotkey));


    result.push_back('}');
    return result;
}

bool hmc_shell_util::getShortcutLink(std::wstring _lnkPath, std::wstring *path, int64_t *showCmd, std::wstring *args, std::wstring *desc, std::wstring *icon, int64_t *iconIndex, std::wstring *cwd, int64_t *hotkey)
{
    bool result = false;
    std::wstring lnkPath;

    if (hmc_string_util::includes(_lnkPath, '\0'))
    {
        lnkPath = std::wstring(_lnkPath.begin(), _lnkPath.end());
        hmc_string_util::formatCStr(lnkPath);
    }
    else
    {
        lnkPath = _lnkPath;
    }

    IShellLinkW *GetShellLinkContent = nullptr;

    auto temp_coinit = ::CoInitialize(0);

    if (!SUCCEEDED(temp_coinit))
    {
        return result;
    }

    HMC_FreeAnyAuto(::CoUninitialize(););

    HRESULT CreateHRESULT = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&GetShellLinkContent);

    if (SUCCEEDED(CreateHRESULT))
    {
        IPersistFile *PersistFile;
        CreateHRESULT = GetShellLinkContent->QueryInterface(IID_IPersistFile, (LPVOID *)&PersistFile);

        if (SUCCEEDED(CreateHRESULT))
        {
            CreateHRESULT = PersistFile->Load(lnkPath.c_str(), STGM_READ);

            if (SUCCEEDED(CreateHRESULT))
            {
                if (path != nullptr)
                {
                    WIN32_FIND_DATAW wfd = WIN32_FIND_DATAW();
                    path->resize(MAX_PATH);
                    GetShellLinkContent->GetPath(path->data(), MAX_PATH, &wfd, SLGP_UNCPRIORITY | SLGP_RAWPATH);

                    auto indexOf = hmc_string_util::indexOf(*path, '\0');
                    if (indexOf >= 0)
                    {
                        path->resize(indexOf);
                    }

                    result = true;
                }

                if (showCmd != nullptr)
                {
                    int temp_GetShowCmdNum = 0;
                    GetShellLinkContent->GetShowCmd(&temp_GetShowCmdNum);
                    (*showCmd) = temp_GetShowCmdNum;
                    result = true;
                }

                if (cwd != nullptr)
                {
                    cwd->resize(MAX_PATH);
                    GetShellLinkContent->GetWorkingDirectory(cwd->data(), MAX_PATH);

                    auto indexOf = hmc_string_util::indexOf(*cwd, '\0');
                    if (indexOf >= 0)
                    {
                        cwd->resize(indexOf);
                    }

                    result = true;
                }

                if (args != nullptr)
                {
                    args->resize(MAX_PATH);
                    GetShellLinkContent->GetArguments(args->data(), MAX_PATH);
                    auto indexOf = hmc_string_util::indexOf(*args, '\0');
                    if (indexOf >= 0)
                    {
                        args->resize(indexOf);
                    }

                    result = true;
                }

                if (desc != nullptr)
                {
                    desc->resize(MAX_PATH);
                    GetShellLinkContent->GetDescription(desc->data(), MAX_PATH);
                    auto indexOf = hmc_string_util::indexOf(*desc, '\0');
                    if (indexOf >= 0)
                    {
                        desc->resize(indexOf);
                    }

                    result = true;
                }

                if (icon != nullptr)
                {

                    int piIcon = 0;
                    icon->resize(MAX_PATH);
                    GetShellLinkContent->GetIconLocation(icon->data(), MAX_PATH, &piIcon);

                    auto indexOf = hmc_string_util::indexOf(*icon, '\0');
                    if (indexOf >= 0)
                    {
                        icon->resize(indexOf);
                    }

                    result = true;

                    if (iconIndex != nullptr)
                    {
                        (*iconIndex) = piIcon;
                    }
                }

                if (hotkey != nullptr)
                {
                    WORD pwHotkey = 0;
                    GetShellLinkContent->GetHotkey(&pwHotkey);
                    result = true;

                    (*hotkey) = pwHotkey;
                }
            }

            PersistFile->Release();
        }
    }

    if (GetShellLinkContent != NULL)
    {
        GetShellLinkContent->Release();
    }

    return result;
}

hmc_shell_util::chShortcutLinkItem hmc_shell_util::getShortcutLink(std::wstring lnkPath)
{

    auto lnkItem = chShortcutLinkItem();

    IShellLinkW *GetShellLinkContent = nullptr;

    getShortcutLink(lnkPath, &lnkItem.path, &lnkItem.showCmd, &lnkItem.args, &lnkItem.desc, &lnkItem.icon, &lnkItem.iconIndex, &lnkItem.cwd, &lnkItem.hotkey);

    return lnkItem;
}

class CComInterfaceReleaser
{
public:
    explicit CComInterfaceReleaser(IUnknown *i) : _i(i) {}
    ~CComInterfaceReleaser()
    {
        if (_i)
            _i->Release();
    }

private:
    IUnknown *_i;
};

struct ComInitializer
{
    bool isValid;
    ComInitializer()
    {
        const auto result = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        isValid = SUCCEEDED(result);
    }

    ~ComInitializer()
    {
        if (isValid)
            ::CoUninitialize();
    }
};

class CItemIdArrayReleaser
{
public:
    explicit CItemIdArrayReleaser(const std::vector<ITEMIDLIST *> &idArray) : _array(idArray) {}
    ~CItemIdArrayReleaser()
    {
        for (ITEMIDLIST *item : _array)
            CoTaskMemFree(item);
    }

    CItemIdArrayReleaser &operator=(const CItemIdArrayReleaser &) = delete;

private:
    const std::vector<ITEMIDLIST *> &_array;
};

bool prepareContextMenuForObjects(std::vector<std::wstring> objects, void *parentWindow, HMENU &hmenu, IContextMenu *&imenu)
{
    ComInitializer comInitializer;

    if (objects.empty())
        return false;

    std::vector<ITEMIDLIST *> ids;
    std::vector<LPCITEMIDLIST> relativeIds;
    IShellFolder *ifolder = 0;
    for (size_t i = 0; i < objects.size(); ++i)
    {
        std::wstring filePath = std::wstring(objects[i]);

        hmc_util::replaceAll(filePath, L"/", L"\\");

        ids.push_back(0);
        HRESULT result = SHParseDisplayName(filePath.c_str(), 0, &ids.back(), 0, 0);
        if (!SUCCEEDED(result) || !ids.back())
        {
            ids.pop_back();
            continue;
        }

        relativeIds.push_back(0);
        result = SHBindToParent(ids.back(), IID_IShellFolder, (void **)&ifolder, &relativeIds.back());
        if (!SUCCEEDED(result) || !relativeIds.back())
            relativeIds.pop_back();
        else if (i < objects.size() - 1 && ifolder)
        {
            ifolder->Release();
            ifolder = nullptr;
        }
    }

    CItemIdArrayReleaser arrayReleaser(ids);

    imenu = 0;
    HRESULT result = ifolder->GetUIObjectOf((HWND)parentWindow, (UINT)relativeIds.size(), (const ITEMIDLIST **)relativeIds.data(), IID_IContextMenu, 0, (void **)&imenu);
    if (!SUCCEEDED(result) || !imenu)
        return false;

    hmenu = CreatePopupMenu();
    if (!hmenu)
        return false;
    return (SUCCEEDED(imenu->QueryContextMenu(hmenu, 0, 1, 0x7FFF, CMF_NORMAL)));
}

// 此 函数传入非法地址会出现错误
bool hmc_shell_util::showContextMenu(HWND hwnd, const std::vector<std::wstring> filePathList, int x, int y)
{

    ComInitializer comInitializer;

    IContextMenu *imenu = 0;
    HMENU hMenu = NULL;
    if (!prepareContextMenuForObjects(filePathList, hwnd, hMenu, imenu) || !hMenu || !imenu)
        return false;

    CComInterfaceReleaser menuReleaser(imenu);

    int command = ::TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, x, y, static_cast<HWND>(hwnd), NULL);
    if (command > 0)
    {

        int command_temp = command - 1;
        CMINVOKECOMMANDINFOEX info = {0};
        info.cbSize = sizeof(info);
        info.hwnd = static_cast<HWND>(hwnd);
        info.lpVerb = MAKEINTRESOURCEA(command - 1);
        info.nShow = SW_NORMAL;
        imenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
    }

    ::DestroyMenu(hMenu);

    return command > 0;
}

bool hmc_shell_util::showContextMenu(HWND hwnd, std::wstring file_Path, int x, int y)
{

    std::wstring filePath = std::wstring(file_Path.begin(), file_Path.end());
    hmc_util::replaceAll(filePath, L"/", L"\\");

    auto temp_coinit = ::CoInitialize(NULL);

    IShellItem *pItem;
    HRESULT hr = ::SHCreateItemFromParsingName(filePath.c_str(), NULL, IID_PPV_ARGS(&pItem));
    if (FAILED(hr))
    {
        return false;
    }

    IContextMenu *pContextMenu;
    hr = pItem->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&pContextMenu));
    if (FAILED(hr))
    {
        pItem->Release();
        return false;
    }

    HMENU hMenu = ::CreatePopupMenu();
    if (hMenu == NULL)
    {
        pContextMenu->Release();
        pItem->Release();
        return false;
    }

    hr = pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL);

    if (FAILED(hr))
    {
        ::DestroyMenu(hMenu);
        pContextMenu->Release();
        pItem->Release();
        return false;
    }

    int command = ::TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, x, y, static_cast<HWND>(hwnd), NULL);
    if (command > 0)
    {
        // if ((command & 0xF000) == 0xF000)
        //{
        //     ::DestroyMenu(hMenu);
        //     return command & 0x0FFF;
        // }

        int command_temp = command - 1;
        CMINVOKECOMMANDINFOEX info = {0};
        info.cbSize = sizeof(info);
        info.hwnd = static_cast<HWND>(hwnd);
        info.lpVerb = MAKEINTRESOURCEA(command - 1);
        info.nShow = SW_NORMAL;
        pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
    }

    // 释放资源
    ::DestroyMenu(hMenu);
    pContextMenu->Release();
    pItem->Release();

    if (temp_coinit > 0)
    {
        ::CoUninitialize();
    }

    return command > 0;
}

bool hmc_shell_util::setShortcutLink(std::wstring lnkPath, chShortcutLinkItem shortcutLinkItem)
{
    if (!shortcutLinkItem.isValid())
    {
        return false;
    }

    std::wstring FilePath = shortcutLinkItem.path;
    std::wstring work_dir = shortcutLinkItem.cwd;
    std::wstring desc = shortcutLinkItem.desc;
    std::wstring args = shortcutLinkItem.args;
    int64_t iShowCmd = shortcutLinkItem.showCmd;
    std::wstring icon = shortcutLinkItem.icon;
    int64_t iconIndex = shortcutLinkItem.iconIndex;

    return setShortcutLink(lnkPath, FilePath, work_dir, desc, args, static_cast<DWORD>(iShowCmd), icon, static_cast<DWORD>(iconIndex));
}

/**
 * @brief 判断是否存在尺寸铺满整个屏幕的软件
 *
 * @return true
 * @return false
 */
bool hmc_shell_util::isFocusWindowFullScreen()
{
    // 全屏应用必定是一个前台窗口
    HWND focusWindow = ::GetForegroundWindow();

    if (focusWindow == NULL)
    {
        return false;
    }

    // 获取活动窗口的位置信息
    RECT windowRect = {0};
    ::GetWindowRect(focusWindow, &windowRect);

    // 获取包含活动窗口的显示器句柄
    HMONITOR monitor = ::MonitorFromWindow(focusWindow, MONITOR_DEFAULTTONEAREST);

    // 获取主显示信息
    MONITORINFO monitorInfo = {sizeof(MONITORINFO)};
    ::GetMonitorInfo(monitor, &monitorInfo);

    // 获取屏幕的尺寸
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    if (windowRect.left <= 0 && windowRect.top <= 0 &&
        windowRect.right >= screenWidth && windowRect.bottom >= screenHeight &&
        monitorInfo.rcMonitor.left == 0 && monitorInfo.rcMonitor.top == 0 &&
        monitorInfo.rcMonitor.right == screenWidth && monitorInfo.rcMonitor.bottom == screenHeight)
    {
        return !hmc_windows_util::isDesktopWindow(focusWindow);
    }

    return false;
}

bool hmc_shell_util::isFullScreen()
{
    bool result = false;

    QUERY_USER_NOTIFICATION_STATE state;
    HRESULT hr = ::SHQueryUserNotificationState(&state);
    if (hr != S_OK)
        return false;

    switch (state)
    {

    // 全屏应用程序 第三方桌面可能也会导致识别错误
    case QUNS_BUSY:
        return isFocusWindowFullScreen();
    // Direct3D 应用程序正在运行) 全屏 (独占模式。
    case QUNS_RUNNING_D3D_FULL_SCREEN:
    // 用户已激活 Windows 演示文稿设置以阻止通知和弹出消息。
    case QUNS_PRESENTATION_MODE:
        return true;

    // 显示屏幕保护程序、锁定计算机或正在进行非活动快速用户切换会话。
    case QUNS_NOT_PRESENT:
    // 找不到其他状态，可以自由发送通知。
    case QUNS_ACCEPTS_NOTIFICATIONS:
    //  当前用户处于“静默时间”，这是新用户首次登录到其帐户后的第一小时
    case QUNS_QUIET_TIME:
    // Windows 应用商店应用正在运行
    case QUNS_APP:
        return false;
    };

    return false;
}

long long int hmc_shell_util::getTimestamp(bool is_Millisecond)
{

    SYSTEMTIME sys;
    ::GetLocalTime(&sys);
    return getTimestamp(sys, is_Millisecond);
}

long long int hmc_shell_util::getTimestamp(const SYSTEMTIME &st, bool is_Millisecond)
{
    FILETIME ft;
    ::SystemTimeToFileTime(&st, &ft);

    // 将 FILETIME 转换为 64 位整数，表示 100 毫微秒为单位的时间数
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // 将时间数转换为毫秒
    return static_cast<long long int>(is_Millisecond ? uli.QuadPart / 10000 : uli.QuadPart);
}

int64_t hmc_shell_util::getSystemIdleTime()
{

    LASTINPUTINFO lpi;
    lpi.cbSize = sizeof(lpi);
    ::GetLastInputInfo(&lpi);
    unsigned long long datetime = ::GetTickCount64() - lpi.dwTime;
    int64_t SystemIdleTime = datetime;

    return SystemIdleTime;
}

bool hmc_shell_util::copyFile(const std::wstring &filePath, const std::wstring &cloneFilePath, bool isShowConfirm, bool isShow, bool isShowProgress)
{
    SHFILEOPSTRUCTW fileOP = {0};
    ::ZeroMemory(&fileOP, sizeof(SHFILEOPSTRUCTW));
    fileOP.wFunc = FO_COPY;
    fileOP.pFrom = filePath.c_str();
    fileOP.pTo = cloneFilePath.c_str();

    fileOP.fFlags = (isShowConfirm ? 0 : (isShowProgress ? 0 : FOF_SILENT)) | FOF_NOCONFIRMATION | (isShow ? 0 : FOF_NO_UI);
    int fileOperation = ::SHFileOperationW(&fileOP);

    return fileOperation == 0;
}

bool hmc_shell_util::moveFile(const std::wstring &filePath, const std::wstring &cloneFilePath, bool isShowConfirm, bool isShow, bool isShowProgress)
{
    SHFILEOPSTRUCTW fileOP = {0};
    ::ZeroMemory(&fileOP, sizeof(SHFILEOPSTRUCTW));
    fileOP.wFunc = FO_MOVE;
    fileOP.pFrom = filePath.c_str();
    fileOP.pTo = cloneFilePath.c_str();
    fileOP.fFlags = (isShowConfirm ? 0 : (isShowProgress ? 0 : FOF_SILENT)) | FOF_NOCONFIRMATION | (isShow ? 0 : FOF_NO_UI);
    int fileOperation = ::SHFileOperationW(&fileOP);
    return fileOperation == 0;
}

bool hmc_shell_util::SetFolderIcon(const std::wstring &folderPath, const std::wstring &iconPath, int iconIndex)
{

    // 设置文件夹属性为只读，保护 desktop.ini 文件
    DWORD folderAttributes = ::GetFileAttributesW(folderPath.c_str());
    if (folderAttributes == INVALID_FILE_ATTRIBUTES)
        return false;

    if ((folderAttributes & FILE_ATTRIBUTE_READONLY) != FILE_ATTRIBUTE_READONLY)
        ::SetFileAttributesW(folderPath.c_str(), folderAttributes | FILE_ATTRIBUTE_READONLY);

    // 构建 desktop.ini 文件路径
    std::wstring desktopIniPath = std::wstring(folderPath.begin(), folderPath.end());
    desktopIniPath.append(L"\\desktop.ini");
    hmc_string_util::removeMatchChar(&desktopIniPath, '\0');

    // 写入 desktop.ini 文件内容
    std::wstring iniContent = L"[.ShellClassInfo]\nIconResource=" + iconPath + L"," + std::to_wstring(iconIndex) + L"\n";

    hmc_string_util::removeMatchChar(&iniContent, '\0');

    if (!::WritePrivateProfileStringW(L".ShellClassInfo", L"IconResource", (iconPath + L"," + std::to_wstring(iconIndex)).c_str(), desktopIniPath.c_str()))
        return false;

    // 设置 desktop.ini 文件属性为隐藏和系统
    ::SetFileAttributesW(desktopIniPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    // 将文件夹的只读属性恢复
    ::SetFileAttributesW(folderPath.c_str(), folderAttributes);

    // 使文件夹的缩略图设置生效
    ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return true;
}

bool hmc_shell_util::powerControl(DWORD dwReason, bool aims)
{
    if (dwReason != EWX_LOGOFF && dwReason != EWX_REBOOT && dwReason != EWX_SHUTDOWN)
        return FALSE;
    OSVERSIONINFO osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    dwReason |= (aims != FALSE) ? EWX_FORCE : EWX_FORCEIFHUNG;
    //::InitiateSystemShutdownEx(
    //    nullptr,             // 指定计算机名，nullptr表示本地计算机
    //    L"Shutting down...", // 关机消息
    //    0,                   // 延迟时间（秒）
    //    TRUE,                // 强制关闭应用程序
    //    TRUE,                // 关闭电源
    //    SHTDN_REASON_MAJOR_OTHER // 关机原因代码
    //);
    return ::ExitWindowsEx(dwReason, 0);
}

void hmc_shell_util::powerControl(int flage)
{
    switch (flage)
    {
    case 1001: // 关机
        powerControl(EWX_SHUTDOWN, true);
        break;
    case 1002: // 重启
        powerControl(EWX_REBOOT, true);
        break;
    case 1003: // 注销
        powerControl(EWX_LOGOFF, false);
        break;
    case 1005: // 锁定
        ::LockWorkStation();
        break;
    case 1006: // 关闭显示器
        ::SendMessage(::FindWindow(0, 0), WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        break;
    case 1007: // 打开显示器
        ::SendMessage(::FindWindow(0, 0), WM_SYSCOMMAND, SC_MONITORPOWER, -1);
        break;
    }
}

void hmc_shell_util::beep()
{
    ::MessageBeep(MB_OK);
}

bool hmc_shell_util::Symlink::isHardLink(const std::wstring &filePath)
{

    HANDLE fileHandle = ::CreateFileW(
        filePath.c_str(),
        0,                     // 不需要任何访问权限
        FILE_SHARE_READ,       // 允许其他进程读取文件
        NULL,                  // 不需要安全描述符
        OPEN_EXISTING,         // 文件必须存在
        FILE_ATTRIBUTE_NORMAL, // 常规文件
        NULL                   // 没有模板文件
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    BY_HANDLE_FILE_INFORMATION fileInfo;
    if (!::GetFileInformationByHandle(fileHandle, &fileInfo))
    {
        ::CloseHandle(fileHandle);
        return false;
    }

    ::CloseHandle(fileHandle);

    // 不可能小于1 因为当前的文件就是其中一个
    return fileInfo.nNumberOfLinks > 1;
}

std::vector<std::wstring> hmc_shell_util::Symlink::getHardLinkList(const std::wstring &filePath)
{
    std::vector<std::wstring> hardLinks;

    std::vector<wchar_t> buffer;

    if (filePath.size() < 3)
    {
        return hardLinks;
    }

    // 缓冲区大小 重置值
    size_t re_length = MAX_PATH + 1;
    // 缓冲区大小 获取值
    DWORD buff_size = MAX_PATH + 1;

    buffer.reserve(re_length);
    buffer.resize(re_length);

    bool next_read = false;

    HANDLE hFind = ::FindFirstFileNameW(filePath.c_str(), 0, &buff_size, &buffer[0]);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        // 使用了长路径 最长32768个
        if (::GetLastError() == ERROR_MORE_DATA)
        {
            re_length = 32768;
            buffer.reserve(re_length);
            buff_size = static_cast<DWORD>(re_length);
            hFind = ::FindFirstFileNameW(filePath.c_str(), 0, &buff_size, &buffer[0]);

            // 无解
            if (hFind == INVALID_HANDLE_VALUE)
            {
                return hardLinks;
            }
        }
        else
            return hardLinks;
    }

    std::shared_ptr<void> open_hkey_close_key(nullptr, [&](void *)
                                              { ::FindClose(hFind); });

    std::wstring driveLetter = std::wstring();
    // \\\\?\\  前缀
    if (filePath[0] == '\\' && filePath.size() > 6)
    {
        driveLetter.push_back(filePath[4]);
        driveLetter.push_back(filePath[5]);
    }
    else
    {
        driveLetter.push_back(filePath[0]);
        driveLetter.push_back(filePath[1]);
    }

    do
    {

        if (buff_size > 0)
        {

            std::wstring temp = std::wstring(driveLetter.begin(), driveLetter.end());

            for (size_t i = 0; i < buff_size; i++)
            {
                auto it = buffer[i];
                if (it != L'\0')
                    temp.push_back(it);
            }

            // temp.push_back(L'\0');

            hardLinks.push_back(temp);
        }

        buff_size = static_cast<DWORD>(re_length);
        next_read = ::FindNextFileNameW(hFind, &buff_size, &buffer[0]);

        if (!next_read)
        {
            // buff_size不等于0 但是提前停止了 那么意味着 此错误是 可能是因为使用了长路径 最长32768个

            if (buff_size > 1 && buff_size != re_length && ::GetLastError() == ERROR_MORE_DATA)
            {
                re_length = 32768;
                buffer.reserve(re_length);
                buff_size = static_cast<DWORD>(re_length);
                buffer.resize(re_length);

                // 重试
                next_read = ::FindNextFileNameW(hFind, &buff_size, &buffer[0]);
                if (next_read)
                {
                    return hardLinks;
                }
            }
            else
            {
                return hardLinks;
            }
        }

    } while (next_read);

    return hardLinks;
}

bool hmc_shell_util::Symlink::isLink(const std::wstring &Path)
{
    DWORD dwAttr = ::GetFileAttributesW(Path.c_str());
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return dwAttr & FILE_ATTRIBUTE_REPARSE_POINT;
}

bool hmc_shell_util::Symlink::isSymbolicDirLink(const std::wstring &Path)
{
    bool result = false;
    if (!(getFileAttributesInfo(Path) & FILE_ATTRIBUTE_DIRECTORY))
    {
        return result;
    }
    result = getLinkType(Path) == LINK_TYPE::JUNCTION_POINT_DIR;
    return result;
}

bool hmc_shell_util::Symlink::isDirLink(const std::wstring &Path)
{
    bool result = false;
    if (!(getFileAttributesInfo(Path) & FILE_ATTRIBUTE_DIRECTORY))
    {
        return result;
    }
    result = getLinkType(Path) == LINK_TYPE::SOFT_LINK_DIR;
    return result;
}

std::wstring hmc_shell_util::Symlink::getSymbolicLinkTarget(const std::wstring &symlinkPath)
{
    HANDLE hFile = ::CreateFileW(symlinkPath.c_str(), 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE || hFile == NULL)
    {
        return L"";
    }

    std::vector<wchar_t> targetPath(MAX_PATH + 1);

    DWORD pathSize = ::GetFinalPathNameByHandleW(hFile, targetPath.data(), MAX_PATH, VOLUME_NAME_DOS);
    if (pathSize == 0)
    {
        ::CloseHandle(hFile);
        return L"";
    }

    // '\\\\?\\F:\\source\\module\\hmc-win32\\source\\h2c-win32\\source\\node\\debug.js'
    std::wstring finalPath = std::wstring(targetPath.data(), pathSize);

    const std::wstring prefix = L"\\\\?\\";

    if (finalPath.find(prefix) == 0)
    {
        finalPath.erase(0, prefix.size());
    }

    ::CloseHandle(hFile);
    return finalPath;
}

bool hmc_shell_util::Symlink::createDirSymlink(const std::wstring &targetPath, const std::wstring &sourcePath)
{
    return ::CreateSymbolicLinkW(targetPath.c_str(), sourcePath.c_str(), 0x1);
}

hmc_shell_util::Symlink::LINK_TYPE hmc_shell_util::Symlink::getLinkType(const std::wstring &path)
{
    DWORD attributes = ::GetFileAttributesW(path.c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return LINK_TYPE::UNKNOWN;
    }

    // 检查是否是符号链接或目录连接点
    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {

        HANDLE fileHandle = ::CreateFileW(
            path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            NULL);

        std::shared_ptr<void> _shared_close_Handle_(nullptr, [&](void *)
                                                    {
            if (fileHandle != INVALID_HANDLE_VALUE) {
                ::CloseHandle(fileHandle);
            } });

        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            // IOCTL重解析
            BYTE reparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
            DWORD bytesReturned;

            if (::DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT, NULL, 0,
                                  reparseBuffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, NULL))
            {
                ch_copy_REPARSE_DATA_BUFFER *reparseData = (ch_copy_REPARSE_DATA_BUFFER *)reparseBuffer;

                if (reparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK)
                {
                    // 符号链接
                    return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? LINK_TYPE::SOFT_LINK_DIR : LINK_TYPE::SOFT_LINK_FILE;
                }
                else if (reparseData->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
                {
                    // 目录连接点
                    return LINK_TYPE::JUNCTION_POINT_DIR;
                }
            }
        }
    }

    // 判断是否是硬链接
    HANDLE fileHandle = ::CreateFileW(
        path.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    std::shared_ptr<void> _shared_close_Handle_(nullptr, [&](void *)
                                                {
        if (fileHandle != INVALID_HANDLE_VALUE) {
            ::CloseHandle(fileHandle);
        } });

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        BY_HANDLE_FILE_INFORMATION fileInfo;
        if (::GetFileInformationByHandle(fileHandle, &fileInfo))
        {
            if (fileInfo.nNumberOfLinks > 1)
            {
                // 硬链接
                return LINK_TYPE::HARD_LINK_FILE;
            }
        }
    }

    return LINK_TYPE::UNKNOWN;
}

bool hmc_shell_util::Symlink::createSymlink(const std::wstring &targetPath, const std::wstring &sourcePath)
{
    return ::CreateSymbolicLinkW(targetPath.c_str(), sourcePath.c_str(), 0x1);
}

bool hmc_shell_util::Symlink::createHardLink(const std::wstring &targetPath, const std::wstring &sourcePath)
{
    return ::CreateHardLinkW(targetPath.c_str(), sourcePath.c_str(), NULL);
}

bool hmc_shell_util::Symlink::createSymbolicLink(const std::wstring &targetPath, const std::wstring &sourcePath)
{
    return ::CreateSymbolicLinkW(targetPath.c_str(), sourcePath.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY);
}

int GetThumbnail::GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
    UINT num = 0;  // number of image encoders
    UINT size = 0; // size of the image encoder array in bytes

    Gdiplus::GetImageEncodersSize(&num, &size);

    if (size == NULL)
        return -1; // Failure

    auto pImageCodecInfo = VcRef<Gdiplus::ImageCodecInfo>(size);

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (::wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            return j; // Success
        }
    }

    return -1; // Failure
}

void GetThumbnail::WriteToImageByte(Gdiplus::Bitmap *image, LPCWSTR formatMimeType, std::vector<BYTE> &result)
{

    CLSID myClsId;
    int retVal = GetEncoderClsid(formatMimeType, &myClsId);
    if (retVal == -1)
        return;

    IStream *istream = nullptr;
    if (::CreateStreamOnHGlobal(NULL, TRUE, &istream) != 0)
        return;

    ((Gdiplus::Bitmap *)image)->Save(istream, &myClsId, nullptr);

    HGLOBAL hg = NULL;
    if (::GetHGlobalFromStream(istream, &hg) != S_OK)
        return;

    size_t bufsize = ::GlobalSize(hg);
    result.resize(bufsize);

    LPVOID pimage = ::GlobalLock(hg);

    if (!pimage)
    {
        result.clear();
        result.resize(0);
        return;
    }

    ::memcpy(&result[0], pimage, bufsize);

    istream->Release();

    ::GlobalUnlock(hg);

    return;
}

void GetThumbnail::GetBitmapPixel(HBITMAP hbitmap, std::vector<BYTE> &result, Gdiplus::PixelFormat format, LPCWSTR formatMimeType)
{

    PALETTEENTRY pat = {255, 255, 255, 0};
    LOGPALETTE logpalt = {0, 1, pat};
    HPALETTE hpalette = ::CreatePalette(&logpalt);

    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hbitmap, hpalette);
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    Gdiplus::BitmapData bd;
    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, bitmap->GetPixelFormat(), &bd);

    std::unique_ptr<Gdiplus::Bitmap> bitmapWithAlpha(new Gdiplus::Bitmap(bd.Width, bd.Height, bd.Stride, format, (BYTE *)bd.Scan0));

    bitmap->UnlockBits(&bd);

    WriteToImageByte(bitmapWithAlpha.get(), formatMimeType, result);
}

std::vector<std::uint8_t> hmc_shell_util::getThumbnailBuffer(std::wstring input, int nSize)
{
    std::vector<std::uint8_t> result;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != 0)
    {
        return result;
    }

    HMC_FreeAnyAuto({
        Gdiplus::GdiplusShutdown(gdiplusToken);
    });

    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        HMC_FreeAnyAuto({ ::CoUninitialize(); });

        IShellItemImageFactory *pImageFactory;

        hr = ::SHCreateItemFromParsingName(input.c_str(), nullptr, IID_PPV_ARGS(&pImageFactory));

        if (SUCCEEDED(hr))
        {
            SIZE size = {nSize, nSize};

            HMC_FreeAnyAuto({ pImageFactory->Release(); });

            HBITMAP hbitmap;
            hr = pImageFactory->GetImage(size, SIIGBF_BIGGERSIZEOK | SIIGBF_ICONONLY | SIIGBF_SCALEUP, &hbitmap);
            if (SUCCEEDED(hr))
            {
                HMC_FreeAnyAuto({ ::DeleteObject(hbitmap); });

                PALETTEENTRY pat = {255, 255, 255, 0};
                LOGPALETTE logpalt = {0, 1, pat};
                HPALETTE hpalette = ::CreatePalette(&logpalt);

                Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hbitmap, hpalette);

                if (bitmap != NULL)
                {
                    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
                    Gdiplus::BitmapData bd;
                    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, bitmap->GetPixelFormat(), &bd);
                    std::unique_ptr<Gdiplus::Bitmap> bitmapWithAlpha(new Gdiplus::Bitmap(bd.Width, bd.Height, bd.Stride, PixelFormat32bppARGB, (BYTE *)bd.Scan0));
                    bitmap->UnlockBits(&bd);
                    GetThumbnail::WriteToImageByte(bitmapWithAlpha.get(), L"image/png", result);
                }
            }
        }
    }

    return result;
}

bool hmc_shell_util::getThumbnailToFile(std::wstring input, std::wstring output, int nSize)
{

    auto data = getThumbnailBuffer(input, nSize);

    if (!data.empty())
    {
        std::ofstream OutFile(output.c_str(), std::ios::out | std::ios::binary);
        OutFile.write(reinterpret_cast<char *>(&data[0]), data.size());

        if (OutFile.fail())
        {
            return false;
        }

        OutFile.close();
        return true;
    }

    return false;
}

bool hmc_shell_util::Startup::set(const std::wstring &key, const std::wstring &execPath, const std::wstring &cmd)
{
    bool result = false;
    auto ExecPath = std::wstring();

    ExecPath.reserve(execPath.size() + cmd.size() + 64);
    if (cmd.size() >= 1)
    {
        ExecPath.push_back('"');
        ExecPath.append(execPath);
        hmc_util::removeAllCharPtr(ExecPath, '\0');
        ExecPath.push_back('"');

        ExecPath.push_back(' ');
        ExecPath.append(cmd);
    }
    else
    {
        ExecPath.append(execPath);
    }

    result = hmc_registr_util::SetRegistrString(
        HKEY_LOCAL_MACHINE,
        WIN32_RunW,
        key,
        ExecPath, false);

    return result;
}

bool hmc_shell_util::Startup::remove(const std::wstring &key)
{
    bool result = false;
    result = hmc_registr_util::removeRegistrKey(
        HKEY_LOCAL_MACHINE,
        WIN32_RunW,
        key);

    return result;
}

bool hmc_shell_util::Startup::has(const std::wstring &key)
{
    bool result = false;
    result = hmc_registr_util::hasRegistrValue(
        HKEY_LOCAL_MACHINE,
        WIN32_RunW,
        key);
    return result;
}

bool hmc_shell_util::Startup::setOnce(const std::wstring &key, const std::wstring &execPath, const std::wstring &cmd)
{
    bool result = false;
    auto ExecPath = std::wstring();

    ExecPath.reserve(execPath.size() + cmd.size() + 64);
    if (cmd.size() >= 1)
    {
        ExecPath.push_back('"');
        ExecPath.append(execPath);
        hmc_util::removeAllCharPtr(ExecPath, '\0');
        ExecPath.push_back('"');

        ExecPath.push_back(' ');
        ExecPath.append(cmd);
    }
    else
    {
        ExecPath.append(execPath);
    }

    result = hmc_registr_util::SetRegistrString(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceW,
        key,
        ExecPath, false);

    return result;
}

bool hmc_shell_util::Startup::removeOnce(const std::wstring &key)
{
    bool result = false;
    result = hmc_registr_util::removeRegistrKey(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceW,
        key);

    return result;
}

bool hmc_shell_util::Startup::hasOnce(const std::wstring &key)
{
    bool result = false;
    result = hmc_registr_util::hasRegistrValue(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceW,
        key);
    return result;
}

bool hmc_shell_util::Startup::set(const std::string &key, const std::string &execPath, const std::string &cmd)
{
    bool result = false;
    auto ExecPath = std::string();

    ExecPath.reserve(execPath.size() + cmd.size() + 64);
    if (cmd.size() >= 1)
    {
        ExecPath.push_back('"');
        ExecPath.append(execPath);
        ExecPath.push_back('"');

        ExecPath.push_back(' ');
        ExecPath.append(cmd);
    }
    else
    {
        ExecPath.append(execPath);
    }

    result = hmc_registr_util::SetRegistrString(
        HKEY_LOCAL_MACHINE,
        WIN32_RunA,
        key,
        ExecPath, false);

    return result;
}

bool hmc_shell_util::Startup::remove(const std::string &key)
{
    bool result = false;
    result = hmc_registr_util::removeRegistrKey(
        HKEY_LOCAL_MACHINE,
        WIN32_RunA,
        key);

    return result;
}

bool hmc_shell_util::Startup::has(const std::string &key)
{
    bool result = false;
    result = hmc_registr_util::hasRegistrValue(
        HKEY_LOCAL_MACHINE,
        WIN32_RunA,
        key);
    return result;
}

bool hmc_shell_util::Startup::setOnce(const std::string &key, const std::string &execPath, const std::string &cmd)
{
    bool result = false;
    auto ExecPath = std::string();

    ExecPath.reserve(execPath.size() + cmd.size() + 64);
    if (cmd.size() >= 1)
    {
        ExecPath.push_back('"');
        ExecPath.append(execPath);
        ExecPath.push_back('"');

        ExecPath.push_back(' ');
        ExecPath.append(cmd);
    }
    else
    {
        ExecPath.append(execPath);
    }

    result = hmc_registr_util::SetRegistrString(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceA,
        key,
        ExecPath, false);

    return result;
}

bool hmc_shell_util::Startup::removeOnce(const std::string &key)
{
    bool result = false;
    result = hmc_registr_util::removeRegistrKey(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceA,
        key);

    return result;
}

bool hmc_shell_util::Startup::hasOnce(const std::string &key)
{
    bool result = false;
    result = hmc_registr_util::hasRegistrValue(
        HKEY_LOCAL_MACHINE,
        WIN32_RunOnceA,
        key);
    return result;
}

const wchar_t hmc_shell_util::Startup::WIN32_RunOnceW[50] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
const wchar_t hmc_shell_util::Startup::WIN32_RunW[46] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
const char hmc_shell_util::Startup::WIN32_RunOnceA[50] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
const char hmc_shell_util::Startup::WIN32_RunA[46] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

bool hmc_shell_util::Dialog::SelectFolder(std::vector<std::wstring> &folderPaths, const std::wstring &initRootDir, const std::wstring &initSelectName, const std::wstring &title, const std::wstring &confirmLabel, bool multSelect, bool dontAddToRecent)
{
    bool result = false;

    try
    {
        HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
        {
            return false;
        }

        IFileOpenDialog *pOpenFolderDialog;
        IShellItemArray *pItemArray;
        HWND owner = NULL;

        hr = ::CoCreateInstance(CLSID_FileOpenDialog,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&pOpenFolderDialog));

        if (!pOpenFolderDialog)
        {
            return false;
        }

        if (SUCCEEDED(hr))
        {

            DWORD dwOptions;
            hr = pOpenFolderDialog->GetOptions(&dwOptions);
            if (SUCCEEDED(hr))
            {

                dwOptions |= FOS_PICKFOLDERS;

                if (dontAddToRecent)
                {
                    dwOptions |= FOS_DONTADDTORECENT;
                }

                if (multSelect)
                {
                    dwOptions |= FOS_ALLOWMULTISELECT;
                }

                hr = pOpenFolderDialog->SetOptions(dwOptions);
            }

            // 初始化文件夹
            if (!initRootDir.empty())
            {
                IShellItem *pInitFolder = nullptr;
                hr = ::SHCreateItemFromParsingName(initRootDir.c_str(), NULL, IID_PPV_ARGS(&pInitFolder));

                if (SUCCEEDED(hr))
                {
                    pOpenFolderDialog->SetFolder(pInitFolder);
                    pInitFolder->Release();
                }
            }

            // 预选文件名
            if (!initSelectName.empty())
            {
                pOpenFolderDialog->SetFileName(initSelectName.c_str());
            }

            // 标题
            if (!title.empty())
            {
                pOpenFolderDialog->SetTitle(title.c_str());
            }

            // 确认按钮的名字
            if (!confirmLabel.empty())
            {
                pOpenFolderDialog->SetOkButtonLabel(confirmLabel.c_str());
            }

            // 启动
            hr = pOpenFolderDialog->Show(owner);

            if (SUCCEEDED(hr))
            {
                hr = pOpenFolderDialog->GetResults(&pItemArray);

                if (SUCCEEDED(hr))
                {
                    DWORD itemCount;
                    hr = pItemArray->GetCount(&itemCount);

                    if (SUCCEEDED(hr))
                    {
                        for (DWORD i = 0; i < itemCount; i++)
                        {
                            IShellItem *pItem;
                            hr = pItemArray->GetItemAt(i, &pItem);

                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFolderPath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

                                if (SUCCEEDED(hr))
                                {
                                    folderPaths.push_back(pszFolderPath);
                                    ::CoTaskMemFree(pszFolderPath);
                                    if (!multSelect)
                                    {
                                        break;
                                    }
                                }

                                pItem->Release();
                            }
                        }

                        if (itemCount > 0)
                        {
                            result = true;
                        }
                    }

                    pItemArray->Release();
                }
            }

            pOpenFolderDialog->Release();
        }
    }
    catch (...)
    {
    };
    return result;
}

bool hmc_shell_util::Dialog::SelectFolder(std::wstring &folderPath, const std::wstring &initRootDir, const std::wstring &initSelectName, const std::wstring &title, const std::wstring &confirmLabel, bool dontAddToRecent)
{
    std::vector<std::wstring> FileList;
    bool result = SelectFolder(FileList, initRootDir, initSelectName, title, confirmLabel, false, dontAddToRecent);
    if (!FileList.empty())
    {
        folderPath.append(FileList.at(0));
    }
    return result;
}

bool hmc_shell_util::Dialog::SelectFile(std::vector<std::wstring> &FilePaths, const std::wstring &initRootDir, const std::wstring &initSelectName, const std::wstring &title, const std::wstring &confirmLabel, const std::vector<std::pair<std::wstring, std::wstring>> &filters, bool dontAddToRecent, bool multSelect)
{
    bool result = false;

    try
    {
        HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
        {
            return false;
        }

        IFileOpenDialog *pFileOpenDialog;
        IShellItemArray *pItemArray;
        HWND owner = NULL;

        // 创建文件选择对话框
        hr = ::CoCreateInstance(CLSID_FileOpenDialog,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&pFileOpenDialog));

        if (!pFileOpenDialog)
        {
            return false;
        }

        if (SUCCEEDED(hr))
        {
            // 设置对话框选项
            DWORD dwOptions;
            hr = pFileOpenDialog->GetOptions(&dwOptions);
            if (SUCCEEDED(hr))
            {

                if (dontAddToRecent)
                {
                    dwOptions |= FOS_DONTADDTORECENT; // 不添加到最近的文档
                }
                if (multSelect)
                {

                    dwOptions |= FOS_ALLOWMULTISELECT; // 允许多选
                }

                dwOptions |= FOS_FORCEFILESYSTEM; // 仅允许文件系统对象

                hr = pFileOpenDialog->SetOptions(dwOptions);
            }

            // 设置初始目录
            if (!initRootDir.empty())
            {
                IShellItem *pFolder;
                hr = ::SHCreateItemFromParsingName(initRootDir.c_str(), NULL, IID_PPV_ARGS(&pFolder));
                if (SUCCEEDED(hr))
                {
                    pFileOpenDialog->SetFolder(pFolder);
                    pFolder->Release();
                }
            }

            // 设置初始选择名称
            if (!initSelectName.empty())
            {
                pFileOpenDialog->SetFileName(initSelectName.c_str());
            }

            // 设置对话框标题
            if (!title.empty())
            {
                pFileOpenDialog->SetTitle(title.c_str());
            }

            // 设置确认按钮的自定义标签
            if (!confirmLabel.empty())
            {
                pFileOpenDialog->SetOkButtonLabel(confirmLabel.c_str());
            }

            // 设置文件类型过滤器
            if (!filters.empty())
            {
                std::vector<COMDLG_FILTERSPEC> fileTypes;
                for (const auto &filter : filters)
                {
                    COMDLG_FILTERSPEC spec = {filter.first.c_str(), filter.second.c_str()};
                    fileTypes.push_back(spec);
                }
                pFileOpenDialog->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data());
            }

            // 显示文件选择对话框
            hr = pFileOpenDialog->Show(owner);
            if (SUCCEEDED(hr))
            {
                hr = pFileOpenDialog->GetResults(&pItemArray);

                if (SUCCEEDED(hr))
                {
                    DWORD itemCount;
                    hr = pItemArray->GetCount(&itemCount);

                    if (SUCCEEDED(hr))
                    {
                        for (DWORD i = 0; i < itemCount; i++)
                        {
                            IShellItem *pItem;
                            hr = pItemArray->GetItemAt(i, &pItem);

                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFolderPath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

                                if (SUCCEEDED(hr))
                                {
                                    FilePaths.push_back(pszFolderPath);
                                    ::CoTaskMemFree(pszFolderPath);
                                }

                                pItem->Release();
                            }
                        }

                        if (itemCount > 0)
                        {
                            result = true;
                        }
                    }

                    pItemArray->Release();
                }
            }

            pFileOpenDialog->Release();
        }
    }
    catch (...)
    {
    }

    return result;
}

bool hmc_shell_util::Dialog::SelectFile(std::wstring &FilePath, const std::wstring &initRootDir, const std::wstring &initSelectName, const std::wstring &title, const std::wstring &confirmLabel, const std::vector<std::pair<std::wstring, std::wstring>> &filters, bool dontAddToRecent)
{
    std::vector<std::wstring> FileList;
    bool result = SelectFile(FileList, initRootDir, initSelectName, title, confirmLabel, filters, false, dontAddToRecent);
    if (!FileList.empty())
    {
        FilePath.append(FileList.at(0));
    }
    return result;
}

std::vector<std::wstring> hmc_shell_util::Dialog::SelectFile(const std::vector<std::pair<std::wstring, std::wstring>> &filters, bool multSelect)
{
    std::vector<std::wstring> FileList;
    SelectFile(FileList, L"", L"", L"", L"", filters, false, multSelect);
    return FileList;
}

std::vector<std::wstring> hmc_shell_util::Dialog::SelectFolder(bool multSelect)
{
    std::vector<std::wstring> FolderList;
    SelectFolder(FolderList, L"", L"", L"", L"", false, multSelect);
    return FolderList;
}

bool hmc_shell_util::Dialog::SelectSaveFile(std::wstring &filePath, std::wstring &selectExt, const std::wstring &initFolder, const std::wstring &defaultName, const std::wstring &title, const std::wstring &confirmLabel, const std::vector<std::pair<std::wstring, std::wstring>> &extensionList)
{
    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        return false;
    }

    IFileSaveDialog *pFileSaveDialog;
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void **>(&pFileSaveDialog));

    if (pFileSaveDialog != NULL && SUCCEEDED(hr))
    {
        // 设置初始目录
        if (!initFolder.empty())
        {
            IShellItem *psiFolder = NULL;
            hr = SHCreateItemFromParsingName(initFolder.c_str(), NULL, IID_PPV_ARGS(&psiFolder));
            if (SUCCEEDED(hr))
            {
                pFileSaveDialog->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }

        // 设置默认文件名
        if (!defaultName.empty())
        {
            pFileSaveDialog->SetFileName(defaultName.c_str());
        }

        // 设置对话框标题
        if (!title.empty())
        {
            pFileSaveDialog->SetTitle(title.c_str());
        }

        // 设置确认按钮的自定义标签
        if (!confirmLabel.empty())
        {
            pFileSaveDialog->SetOkButtonLabel(confirmLabel.c_str());
        }

        // 设置文件类型过滤器
        if (!extensionList.empty())
        {
            std::vector<COMDLG_FILTERSPEC> fileTypes;
            for (const auto &filter : extensionList)
            {
                COMDLG_FILTERSPEC spec = {filter.first.c_str(), filter.second.c_str()};
                fileTypes.push_back(spec);
            }

            pFileSaveDialog->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data());
        }

        // 显示对话框
        hr = pFileSaveDialog->Show(NULL);
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileSaveDialog->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {

                if (!extensionList.empty())
                {
                    UINT fileTypeIndex = 0;
                    HRESULT hr = pFileSaveDialog->GetFileTypeIndex(&fileTypeIndex);

                    // 检查 fileTypeIndex 是否有效
                    if (SUCCEEDED(hr) && fileTypeIndex > 0 && fileTypeIndex <= extensionList.size())
                    {

                        std::wstring selectedExtension = extensionList.at(fileTypeIndex - 1).second;
                        selectExt.append(selectedExtension);
                    }
                }

                PWSTR pszFilePath = NULL;
                pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (pszFilePath)
                {
                    filePath.clear();
                    filePath.append(pszFilePath);
                    ::CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileSaveDialog->Release();
    }

    ::CoUninitialize();
    return SUCCEEDED(hr);
}

bool hmc_shell_util::Dialog::SelectSaveFolder(std::wstring &folderPath, const std::wstring &initFolder, const std::wstring &defaultName, const std::wstring &title, const std::wstring &confirmLabel)
{
    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        return false;
    }

    IFileSaveDialog *pFileSaveDialog;
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void **>(&pFileSaveDialog));

    if (pFileSaveDialog != NULL && SUCCEEDED(hr))
    {

        DWORD dwOptions;
        hr = pFileSaveDialog->GetOptions(&dwOptions);

        if (SUCCEEDED(hr))
        {

            dwOptions |= FOS_PICKFOLDERS;
            hr = pFileSaveDialog->SetOptions(dwOptions);
        }

        // 设置初始目录
        if (!initFolder.empty())
        {
            IShellItem *psiFolder = NULL;
            hr = ::SHCreateItemFromParsingName(initFolder.c_str(), NULL, IID_PPV_ARGS(&psiFolder));
            if (SUCCEEDED(hr))
            {
                pFileSaveDialog->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }

        // 设置默认文件名
        if (!defaultName.empty())
        {
            pFileSaveDialog->SetFileName(defaultName.c_str());
        }

        // 设置对话框标题
        if (!title.empty())
        {
            pFileSaveDialog->SetTitle(title.c_str());
        }

        // 设置确认按钮的自定义标签
        if (!confirmLabel.empty())
        {
            pFileSaveDialog->SetOkButtonLabel(confirmLabel.c_str());
        }

        // 显示对话框
        hr = pFileSaveDialog->Show(NULL);
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileSaveDialog->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {

                PWSTR pszFilePath = NULL;
                pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (pszFilePath)
                {
                    folderPath.clear();
                    folderPath.append(pszFilePath);

                    ::CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileSaveDialog->Release();
    }

    ::CoUninitialize();
    return SUCCEEDED(hr);
}

bool hmc_shell_util::Dialog::SelectFolderOld(std::wstring &folderPath, const std::wstring &title)
{

    try
    {
        BROWSEINFO bi = {0};
        bi.lpszTitle = title.empty() ? NULL : title.c_str();
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.hwndOwner = NULL;

        LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
        if (pidl != NULL)
        {
            wchar_t folderPathTemp[MAX_PATH];

            if (::SHGetPathFromIDList(pidl, folderPathTemp))
            {
                ::CoTaskMemFree(pidl);
                folderPath.clear();
                folderPath.append(folderPathTemp);
                return true;
            }
            ::CoTaskMemFree(pidl);
        }
    }
    catch (...)
    {
    }
    return false;
}

DWORD hmc_shell_util::getFileAttributesInfo(const std::wstring &path)
{

    DWORD result = 0x0;

    DWORD dwFileAttributes = ::GetFileAttributesW(path.c_str());

    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
    {
        result = dwFileAttributes;
        return result;
    }

    return result;
}

std::vector<DWORD> hmc_shell_util::getFileAttributesInfoList(const std::wstring &path)
{

    std::vector<DWORD> result;

    const DWORD FILE_ATTRIBUTE_INFO_LIST[20] = {
        FILE_ATTRIBUTE_READONLY,            // 0x00000001 // 文件是只读的
        FILE_ATTRIBUTE_HIDDEN,              // 0x00000002 // 文件是隐藏的
        FILE_ATTRIBUTE_SYSTEM,              // 0x00000004 // 文件是系统文件
        FILE_ATTRIBUTE_DIRECTORY,           // 0x00000010 // 文件是一个目录
        FILE_ATTRIBUTE_ARCHIVE,             // 0x00000020 // 文件将被备份或归档
        FILE_ATTRIBUTE_DEVICE,              // 0x00000040 // 保留，不用于文件
        FILE_ATTRIBUTE_NORMAL,              // 0x00000080 // 文件没有其他属性集
        FILE_ATTRIBUTE_TEMPORARY,           // 0x00000100 // 文件用于临时存储，尽快删除
        FILE_ATTRIBUTE_SPARSE_FILE,         // 0x00000200 // 文件是稀疏文件，节省磁盘空间
        FILE_ATTRIBUTE_REPARSE_POINT,       // 0x00000400 // 文件或目录具有重解析点
        FILE_ATTRIBUTE_COMPRESSED,          // 0x00000800 // 文件或目录是压缩的
        FILE_ATTRIBUTE_OFFLINE,             // 0x00001000 // 文件的数据没有立即可用
        FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, // 0x00002000 // 文件或目录不被内容索引
        FILE_ATTRIBUTE_ENCRYPTED,           // 0x00004000 // 文件或目录是加密的
        FILE_ATTRIBUTE_VIRTUAL,             // 0x00010000 // 保留，供未来使用
        FILE_ATTRIBUTE_INTEGRITY_STREAM,    // 0x00008000 // 文件包含完整性流
        FILE_ATTRIBUTE_NO_SCRUB_DATA,       // 0x00020000 // 文件禁止自动数据擦除功能
        FILE_ATTRIBUTE_EA,                  // 0x00040000 // 文件包含扩展属性 (EA)
        FILE_ATTRIBUTE_PINNED,              // 0x00080000 // 文件为固定文件
        FILE_ATTRIBUTE_UNPINNED             // 0x00100000 // 文件为未固定文件
    };

    DWORD dwFileAttributes = ::GetFileAttributesW(path.c_str());

    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
    {
        result.push_back(dwFileAttributes);
        for (int i = 0; i < sizeof(FILE_ATTRIBUTE_INFO_LIST) / sizeof(FILE_ATTRIBUTE_INFO_LIST[0]); i++)
        {
            if (dwFileAttributes & FILE_ATTRIBUTE_INFO_LIST[i])
            {
                result.push_back(FILE_ATTRIBUTE_INFO_LIST[i]);
            }
        }
    }

    return result;
}

DWORD hmc_shell_util::getFileAttributesInfo(const std::string &path)
{

    DWORD result = 0x0;

    DWORD dwFileAttributes = ::GetFileAttributesA(path.c_str());

    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
    {
        result = dwFileAttributes;
        return result;
    }

    return result;
}

std::vector<DWORD> hmc_shell_util::getFileAttributesInfoList(const std::string &path)
{

    std::vector<DWORD> result;

    const DWORD FILE_ATTRIBUTE_INFO_LIST[20] = {
        FILE_ATTRIBUTE_READONLY,            // 0x00000001 // 文件是只读的
        FILE_ATTRIBUTE_HIDDEN,              // 0x00000002 // 文件是隐藏的
        FILE_ATTRIBUTE_SYSTEM,              // 0x00000004 // 文件是系统文件
        FILE_ATTRIBUTE_DIRECTORY,           // 0x00000010 // 文件是一个目录
        FILE_ATTRIBUTE_ARCHIVE,             // 0x00000020 // 文件将被备份或归档
        FILE_ATTRIBUTE_DEVICE,              // 0x00000040 // 保留，不用于文件
        FILE_ATTRIBUTE_NORMAL,              // 0x00000080 // 文件没有其他属性集
        FILE_ATTRIBUTE_TEMPORARY,           // 0x00000100 // 文件用于临时存储，尽快删除
        FILE_ATTRIBUTE_SPARSE_FILE,         // 0x00000200 // 文件是稀疏文件，节省磁盘空间
        FILE_ATTRIBUTE_REPARSE_POINT,       // 0x00000400 // 文件或目录具有重解析点
        FILE_ATTRIBUTE_COMPRESSED,          // 0x00000800 // 文件或目录是压缩的
        FILE_ATTRIBUTE_OFFLINE,             // 0x00001000 // 文件的数据没有立即可用
        FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, // 0x00002000 // 文件或目录不被内容索引
        FILE_ATTRIBUTE_ENCRYPTED,           // 0x00004000 // 文件或目录是加密的
        FILE_ATTRIBUTE_VIRTUAL,             // 0x00010000 // 保留，供未来使用
        FILE_ATTRIBUTE_INTEGRITY_STREAM,    // 0x00008000 // 文件包含完整性流
        FILE_ATTRIBUTE_NO_SCRUB_DATA,       // 0x00020000 // 文件禁止自动数据擦除功能
        FILE_ATTRIBUTE_EA,                  // 0x00040000 // 文件包含扩展属性 (EA)
        FILE_ATTRIBUTE_PINNED,              // 0x00080000 // 文件为固定文件
        FILE_ATTRIBUTE_UNPINNED             // 0x00100000 // 文件为未固定文件
    };

    DWORD dwFileAttributes = ::GetFileAttributesA(path.c_str());

    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
    {
        result.push_back(dwFileAttributes);
        for (int i = 0; i < sizeof(FILE_ATTRIBUTE_INFO_LIST) / sizeof(FILE_ATTRIBUTE_INFO_LIST[0]); i++)
        {
            auto ofFileAttributes = FILE_ATTRIBUTE_INFO_LIST[i];

            if (dwFileAttributes == ofFileAttributes)
            {
                continue;
            }

            if (dwFileAttributes & ofFileAttributes)
            {
                result.push_back(ofFileAttributes);
            }
        }
    }

    return result;
}

bool hmc_shell_util::SetExtensionIcon(const std::wstring &extensionName, const std::wstring &typeName, const std::wstring &iconPath, const int &iconIndex)
{
    bool result = false;

    std::wstring extensionNamePath = L"Software\\Classes\\" + extensionName;

    std::wstring keyPath = L"Software\\Classes\\" + typeName + L"\\DefaultIcon";

    std::wstring iconPath2 = iconPath + L"," + std::to_wstring(iconIndex);

    hmc_string_util::formatCStr(&extensionNamePath);
    hmc_string_util::formatCStr(&keyPath);
    hmc_string_util::formatCStr(&iconPath2);

    hmc_registr_util::SetRegistrString(HKEY_CURRENT_USER, keyPath, L"", iconPath2, false);
    hmc_registr_util::SetRegistrString(HKEY_CURRENT_USER, extensionNamePath, L"", typeName, false);

    result = hmc_registr_util::hasRegistrValue(HKEY_CURRENT_USER, keyPath, L"");
    result = result && hmc_registr_util::hasRegistrValue(HKEY_CURRENT_USER, extensionNamePath, L"");
    return result;
}

bool hmc_shell_util::SetAppURLProtocol(const std::wstring &protocolName, const std::wstring &appPath, const std::wstring &command)
{
    bool result = false;

    // HKEY_CURRENT_USER\SOFTWARE\Classes\HMMagicPocket
    std::wstring protocolPath = L"Software\\Classes\\" + protocolName;

    hmc_string_util::formatCStr(&protocolPath);

    hmc_registr_util::SetRegistrString(HKEY_CURRENT_USER, protocolPath, L"URL Protocol", L"", false);

    std::wstring URL_LINK = L"URL:" + protocolName;
    hmc_string_util::formatCStr(&URL_LINK);

    hmc_registr_util::SetRegistrString(HKEY_CURRENT_USER, protocolPath, L"", URL_LINK, false);

    std::wstring commandKey = protocolPath + L"\\shell\\open\\command";
    std::wstring commandValue = L"\"" + appPath + L"\" ";

    bool is_add_colon = HS::search(command, LR"(^( +)?".+"( +)?)") == -1;

    if (is_add_colon)
    {
        commandValue.push_back('"');
    }

    commandValue.append(command);

    if (is_add_colon)
    {
        commandValue.push_back('"');
    }

    hmc_string_util::formatCStr(&commandValue);
    hmc_string_util::formatCStr(&commandKey);

    hmc_registr_util::SetRegistrString(HKEY_CURRENT_USER, commandKey, L"", commandValue, false);

    result = hmc_registr_util::hasRegistrValue(HKEY_CURRENT_USER, protocolPath, L"URL Protocol");
    result = result && hmc_registr_util::hasRegistrValue(HKEY_CURRENT_USER, commandKey, L"");
    return result;
}
