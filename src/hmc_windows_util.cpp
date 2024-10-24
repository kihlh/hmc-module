#include "hmc_windows_util.h"
#include "hmc_winapi_util.h"
#include "hmc_path_util.h"
#include "hmc_process_util.h"
#include "hmc_util.h"

#pragma warning(disable : 6258)

const std::map<std::string, int64_t> hmc_windows_util::_$_CLASS_LONG_NAME_LIST_EX = std::map<std::string, int64_t>(
    {
        {"WS_EX_ACCEPTFILES", WS_EX_ACCEPTFILES},                 /* 允许窗口接受拖放文件 */
        {"WS_EX_APPWINDOW", WS_EX_APPWINDOW},                     /* 强制将窗口显示在任务栏上 */
        {"WS_EX_CLIENTEDGE", WS_EX_CLIENTEDGE},                   /* 在窗口的工作区边缘显示一个凸起的边框 */
        {"WS_EX_COMPOSITED", WS_EX_COMPOSITED},                   /* 使用双缓冲绘制窗口，减少闪烁（Windows XP和更高版本） */
        {"WS_EX_CONTEXTHELP", WS_EX_CONTEXTHELP},                 /* 在标题栏上显示问号图标，供上下文帮助使用 */
        {"WS_EX_CONTROLPARENT", WS_EX_CONTROLPARENT},             /* 允许子窗口向父窗口发送 WM_COMMAND 消息 */
        {"WS_EX_DLGMODALFRAME", WS_EX_DLGMODALFRAME},             /* 创建带有双层边框的对话框风格窗口 */
        {"WS_EX_LAYERED", WS_EX_LAYERED},                         /* 创建一个分层窗口，可以设置透明度（Windows 2000和更高版本） */
        {"WS_EX_LAYOUTRTL", WS_EX_LAYOUTRTL},                     /* 所有子窗口的布局从右到左，而不是从左到右 */
        {"WS_EX_LEFT", WS_EX_LEFT},                               /* 窗口具有左对齐属性（默认） */
        {"WS_EX_LEFTSCROLLBAR", WS_EX_LEFTSCROLLBAR},             /* 如果水平滚动条存在，则在左侧显示 */
        {"WS_EX_LTRREADING", WS_EX_LTRREADING},                   /* 窗口文本从左到右读取 */
        {"WS_EX_MDICHILD", WS_EX_MDICHILD},                       /* 多文档界面（MDI）子窗口 */
        {"WS_EX_NOACTIVATE", WS_EX_NOACTIVATE},                   /* 创建一个不激活的窗口 */
        {"WS_EX_NOINHERITLAYOUT", WS_EX_NOINHERITLAYOUT},         /* 不继承父窗口的布局方向 */
        {"WS_EX_NOPARENTNOTIFY", WS_EX_NOPARENTNOTIFY},           /* 子窗口被创建或销毁时不向父窗口发送通知 */
        {"WS_EX_NOREDIRECTIONBITMAP", WS_EX_NOREDIRECTIONBITMAP}, /* 阻止窗口内容被缓存在位图中（Windows 2008 和更高版本） */
        {"WS_EX_OVERLAPPEDWINDOW", WS_EX_OVERLAPPEDWINDOW},       /* WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE 的组合 */
        {"WS_EX_PALETTEWINDOW", WS_EX_PALETTEWINDOW},             /* WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST 的组合 */
        {"WS_EX_RIGHT", WS_EX_RIGHT},                             /* 窗口具有右对齐属性 */
        {"WS_EX_RIGHTSCROLLBAR", WS_EX_RIGHTSCROLLBAR},           /* 如果滚动条存在，则在右侧显示（默认） */
        {"WS_EX_RTLREADING", WS_EX_RTLREADING},                   /* 窗口文本从右到左读取 */
        {"WS_EX_STATICEDGE", WS_EX_STATICEDGE},                   /* 创建具有三维边缘风格的窗口 */
        {"WS_EX_TOOLWINDOW", WS_EX_TOOLWINDOW},                   /* 创建一个工具窗口，该窗口不在任务栏显示 */
        {"WS_EX_TOPMOST", WS_EX_TOPMOST},                         /* 窗口保持在其他非顶层窗口之上 */
        {"WS_EX_TRANSPARENT", WS_EX_TRANSPARENT},                 /* 使得该窗口在绘制时，其下方的窗口会首先绘制 */
        {"WS_EX_WINDOWEDGE", WS_EX_WINDOWEDGE}                    /* 在窗口的边缘显示一个凸起的边框 */
    });

const std::map<std::string, int64_t> hmc_windows_util::_$_CLASS_LONG_NAME_LIST = std::map<std::string, int64_t>(
    // 窗口是弹出式的
    {{"WS_POPUP", WS_POPUP},
     // 窗口可见
     {"WS_VISIBLE", WS_VISIBLE},
     // 窗口具有细线边框
     {"WS_BORDER", WS_BORDER},
     // 窗口具有标题栏， (包含 WS_BORDER 样式) 。
     {"WS_CAPTION", WS_CAPTION},
     // 窗口是子窗口。 具有此样式的窗口不能有菜单栏。 此样式不能与 WS_POPUP 样式一起使用。
     {"WS_CHILD", WS_CHILD},
     // 与 WS_CHILD 样式相同。
     {"WS_CHILDWINDOW", WS_CHILDWINDOW},
     // 排除在父窗口内进行绘制时子窗口占用的区域。 创建父窗口时使用此样式
     {"WS_CLIPCHILDREN", WS_CLIPCHILDREN},
     // 窗口具有垂直滚动条。
     {"WS_VSCROLL", WS_VSCROLL},
     // 该窗口是一个重叠的窗口。 与 WS_OVERLAPPEDWINDOW 样式相同
     {"WS_TILEDWINDOW", WS_TILEDWINDOW},
     // 该窗口是一个重叠的窗口。 重叠的窗口带有标题栏和边框。 与 WS_OVERLAPPED 样式相同。
     {"WS_TILED", WS_TILED},
     //  窗口具有调整大小边框。 与 WS_SIZEBOX 样式相同。
     {"WS_THICKFRAME", WS_THICKFRAME},
     // 窗口是一个控件，当用户按下 TAB 键时，该控件可以接收键盘焦点。 按 Tab 键将键盘焦点更改为具有 WS_TABSTOP 样式的下一个控件。可以打开和关闭此样式以更改对话框导航。 若要在创建窗口后更改此样式，请使用 SetWindowLong 函数。 若要使用户创建的窗口和无模式对话框使用制表位，请更改消息循环以调用 IsDialogMessage 函数。
     {"WS_TABSTOP", WS_TABSTOP},
     // 窗口的标题栏上有一个窗口菜单。 还必须指定 WS_CAPTION 样式。
     {"WS_SYSMENU", WS_SYSMENU},
     // 窗口具有大小调整边框。 与 WS_THICKFRAME 样式相同。
     {"WS_SIZEBOX", WS_SIZEBOX},
     // 窗口是弹出窗口。 必须组合 WS_CAPTION 和 WS_POPUPWINDOW 样式，使窗口菜单可见。
     {"WS_POPUPWINDOW", WS_POPUPWINDOW},
     // 窗口是重叠的窗口。 与 WS_TILEDWINDOW 样式相同。
     {"WS_OVERLAPPEDWINDOW", WS_OVERLAPPEDWINDOW},
     // 窗口是重叠的窗口。 重叠的窗口带有标题栏和边框。 与 WS_TILED 样式相同。
     {"WS_OVERLAPPED", WS_OVERLAPPED},
     // 窗口有一个最小化按钮。 不能与 WS_EX_CONTEXTHELP 样式组合使用。 还必须指定 WS_SYSMENU 样式。
     {"WS_MINIMIZEBOX", WS_MINIMIZEBOX},
     // 窗口最初最小化。 与 WS_ICONIC 样式相同。
     {"WS_MINIMIZE", WS_MINIMIZE},
     //  窗口有一个“最大化”按钮。 不能与 WS_EX_CONTEXTHELP 样式组合使用。 还必须指定 WS_SYSMENU 样式。
     {"WS_MAXIMIZEBOX", WS_MAXIMIZEBOX},
     // 窗口最初是最大化的。
     {"WS_ICONIC", WS_ICONIC},
     // 窗口具有水平滚动条
     {"WS_HSCROLL", WS_HSCROLL},
     // 窗口是一组控件的第一个控件。 组由第一个控件及其后定义的所有控件组成，以及具有 WS_GROUP 样式的下一个控件。 每个组中的第一个控件通常具有 WS_TABSTOP 样式，以便用户可以在组之间移动。 用户随后可以使用方向键将键盘焦点从组中的一个控件更改为组中的下一个控件。可以打开和关闭此样式以更改对话框导航。 若要在创建窗口后更改此样式，请使用 SetWindowLong 函数。
     {"WS_GROUP", WS_GROUP},
     // 窗口具有通常与对话框一起使用的样式的边框。 具有此样式的窗口不能具有标题栏。
     {"WS_DLGFRAME", WS_DLGFRAME},
     // 窗口最初处于禁用状态。 已禁用的窗口无法接收用户的输入。 若要在创建窗口后更改此设置，请使用 EnableWindow 函数。
     {"WS_DISABLED", WS_DISABLED},
     // 相对于彼此剪裁子窗口;也就是说，当特定子窗口收到 WM_PAINT 消息时， WS_CLIPSIBLINGS 样式会将所有其他重叠的子窗口剪裁到子窗口的区域之外进行更新。 如果未指定 WS_CLIPSIBLINGS 且子窗口重叠，则当在子窗口的工作区内绘图时，可以在相邻子窗口的工作区内绘制。
     {"WS_CLIPSIBLINGS", WS_CLIPSIBLINGS}});

namespace hmc_windows_util
{
    namespace hmc_define_util
    {

#define HMC_VirtualAlloc(Type, leng) (Type)::VirtualAlloc((LPVOID)NULL, (DWORD)(leng), MEM_COMMIT, PAGE_READWRITE);
#define HMC_VirtualFree(Virtua)                \
    if (Virtua != NULL)                        \
    {                                          \
        ::VirtualFree(Virtua, 0, MEM_RELEASE); \
    }

#define HMC_VirtualFreeAuto(Virtua)                                         \
    std::shared_ptr<void>##Virtua##_Auto_Close_Virtual(nullptr, [&](void *) \
                                                       { \
    if (Virtua != NULL) \
    { \
        ::VirtualFree(Virtua, 0, MEM_RELEASE); \
    } });

#define HMC_LibraryFreeAuto(hDllInst)                                         \
    std::shared_ptr<void>##hDllInst##_Auto_Close_Library(nullptr, [&](void *) \
                                                         { \
    if (hDllInst != NULL) \
    { \
        ::FreeLibrary(hDllInst); \
    } });

#define HMC_EQ_RUN_LAST(size)      \
    if (size == NULL || size <= 0) \
    {                              \
        return ::GetLastError();   \
    }

#define HMC_PUSH_WSRES(WS)    \
    if (style & WS)           \
    {                         \
        result.push_back(WS); \
    }

#define HMC_CASE_PUSH_WSRES(WS) \
    case WS:                    \
        result.append(##WS);    \
        break;

#define STRINGIFY(x) #x

#define HMC_PUSH_WSRES_STR(WS)                                       \
    if (style & WS)                                                  \
    {                                                                \
        result.push_back(hmc_windows_util::winwowLongClass2str(WS)); \
    }

#define HMC_PUSH_WSRES_STR_EX(WS)                                      \
    if (style & WS)                                                    \
    {                                                                  \
        result.push_back(hmc_windows_util::winwowLongClass2strEx(WS)); \
    }

#define HMC_IS_INSIDE(x1, y1, x2, y2, x, y) ((x > x1 && x < x2 && y > y1 && y < y2) ? true : false)

#define HMC_ALL_CATCH_NONE \
    catch (...) {}

#define None std::nullopt

    }

    std::vector<HWND> _$_getSubWindows_temp;
    std::shared_mutex _$_getSubWindows_shared_mutex;
}

static const size_t MAX_BUFF_OVERFLOW = 64;

// 这里写需要预设值的函数
void hmc_windows_util::setWindowIconByExtract(HWND hWnd, std::optional<std::wstring> ExtractFilePath, std::optional<int> Index, std::optional<int> set_type)
{
    // 我用vsdebug的时候发现打开图标的过程是会导致异常的 而不是可以通过 GetLastError 查询不报错的错误
    // 例如用户传入的是个exe 但是调用的是 ico文件方法 将会导致非winapi异常
    try
    {

        // 没有窗口 查找当前进程的主窗口
        if (hWnd == NULL)
        {
            return;
        }

        auto attachToken = hmc_windows_util::AttachThread(hWnd);

        std::wstring filePath = ExtractFilePath.value_or(L"");
        int index = Index.value_or(0);
        int type = set_type.value_or(0);

        if (filePath.empty())
        {
            filePath = hmc_process_util::getProcessFilePath();
        }

        auto Shell32 = hmc_winapi_util::Shell32();
        auto User32 = hmc_winapi_util::User32();

        auto mode = ::GetModuleHandleW(nullptr);

        if (mode != nullptr) {

        HICON hIcon = Shell32.ExtractIconW(nullptr, reinterpret_cast<LPCWSTR>(filePath.c_str()), index);

        if (hIcon == NULL) {
            return;
        }

        if (type == 0)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            User32.SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            User32.UpdateWindow(hWnd);
            return;
        }

        // Titlebar icon: 16x16
        if (type == 1)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            User32.UpdateWindow(hWnd);
            return;
        }

        // Taskbar icon:  32x32
        if (type == 2)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            User32.UpdateWindow(hWnd);
            return;
        }

        }
    }
    HMC_ALL_CATCH_NONE;
}

void hmc_windows_util::setWindowIconByIco(HWND hWnd, std::optional<std::wstring> IcoFilePath, std::optional<int> set_type)
{
    // 我用vsdebug的时候发现打开图标的过程是会导致异常的 而不是可以通过 GetLastError 查询不报错的错误
    // 例如用户传入的是个ico 但是调用的是 exe文件方法 将会导致非winapi异常
    try
    {
        // 没有窗口 查找当前进程的主窗口
        if (hWnd == NULL)
        {
            return;
        }

        if (!IcoFilePath.has_value())
        {
            return;
        }

        auto attachToken = hmc_windows_util::AttachThread(hWnd);

        auto Shell32 = hmc_winapi_util::Shell32();
        auto User32 = hmc_winapi_util::User32();

        int type = set_type.value_or(0);
        std::wstring icoFilePath = IcoFilePath.value_or(L"");

        HICON hWindowIcon = (HICON)::LoadImageW(GetModuleHandle(NULL), icoFilePath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        HICON hWindowIconBig = (HICON)::LoadImageW(GetModuleHandle(NULL), icoFilePath.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);

        HMC_FreeAnyAuto({
            if (hWindowIcon != NULL)
            {
                ::DestroyIcon(hWindowIcon);
            }

            if (hWindowIconBig != NULL)
            {
                ::DestroyIcon(hWindowIconBig);
            }
        });

        if (type == 0)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hWindowIcon);
            User32.SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hWindowIconBig);
            User32.UpdateWindow(hWnd);
            return;
        }

        // Titlebar icon: 16x16
        if (type == 1)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hWindowIcon);
            User32.UpdateWindow(hWnd);
            return;
        }

        // Taskbar icon:  32x32
        if (type == 2)
        {
            User32.SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hWindowIconBig);
            User32.UpdateWindow(hWnd);
            return;
        }
    }
    HMC_ALL_CATCH_NONE;
}

std::variant<std::vector<HWND>, DWORD> hmc_windows_util::getAllWindows(bool is_Windows)
{
    std::vector<HWND> result = {};
    // 获取桌面句柄
    HWND hwnd = ::GetDesktopWindow();

    std::uint64_t _hwnd = static_cast<std::uint64_t>((long long)hwnd);

    HMC_EQ_RUN_LAST(_hwnd);
    int counter = 0;

    hwnd = ::GetWindow(hwnd, GW_CHILD);
    HMC_EQ_RUN_LAST(_hwnd);

    while (hwnd != NULL)
    {
        if (is_Windows && ::IsWindowVisible(hwnd))
        {
            result.push_back(hwnd);
        }
        else
        {
            result.push_back(hwnd);
        }
        hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    if (result.empty())
    {
        return ::GetLastError();
    }

    return result;
}

std::variant<bool, DWORD> hmc_windows_util::setMoveWindow(HWND hwnd, std::optional<int> x, std::optional<int> y, std::optional<int> w, std::optional<int> h)
{

    if (hwnd == NULL)
        return false;

    RECT rect = {0, 0, 0, 0};
    if ((!w.has_value() || !h.has_value()) && !::GetWindowRect(hwnd, &rect))
    {
        DWORD Last_Error_Code = ::GetLastError();
        if (Last_Error_Code != 0)
        {
            return Last_Error_Code;
        }
    }

    int DeviceCapsWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int DeviceCapsHeight = ::GetSystemMetrics(SM_CYSCREEN);

    int Env_height = (DeviceCapsHeight - rect.top) - (DeviceCapsHeight - rect.bottom);
    int Env_width = (DeviceCapsWidth - rect.left) - (DeviceCapsWidth - rect.right);

    int to_x = (x.has_value() ? x.value() : NULL);
    int to_y = (y.has_value() ? y.value() : NULL);
    int to_width = w.value_or(0) < 1 ? Env_width : w.value_or(0);
    int to_height = h.value_or(0) < 1 ? Env_height : h.value_or(0);

    // 重新设置窗口大小
    return ::MoveWindow(hwnd, to_x, to_y, to_width, to_height, true) ? true : ::GetLastError();
}

void hmc_windows_util::setWindowShake(HWND hWnd, int shakeCount, int shakeDistance, int shakeDuration)
{

    std::thread([hWnd, shakeCount, shakeDistance, shakeDuration]() -> void
                {
                    RECT windowRect;
                    ::GetWindowRect(hWnd, &windowRect);

                    int originalX = windowRect.left;
                    int originalY = windowRect.top;

                    for (int i = 0; i < shakeCount; i++)
                    {
                        // 抖动向右
                        ::SetWindowPos(hWnd, NULL, originalX + shakeDistance, originalY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                        ::Sleep(shakeDuration);

                        // 抖动向左
                        ::SetWindowPos(hWnd, NULL, originalX - shakeDistance, originalY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                        ::Sleep(shakeDuration);
                    }

                    // 最后还原窗口位置
                    ::SetWindowPos(hWnd, NULL, originalX, originalY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); })
        .detach();
}

std::variant<std::wstring, DWORD> hmc_windows_util::getWindowTitle(HWND hwnd)
{
    auto attachToken = hmc_windows_util::AttachThread();

    // 计算缓冲区大小

    DWORD buff_size = ::GetWindowTextLengthW(hwnd);

    if (buff_size <= 0)
    {
        if (!attachToken.open(hwnd))
        {
            return ::GetLastError();
        }

        buff_size = ::GetWindowTextLengthW(hwnd);

        if (buff_size <= 0)
        {
            return ::GetLastError();
        }
    }

    std::vector<wchar_t> pszMem(MAX_BUFF_OVERFLOW + buff_size, '\0');

    size_t leng = ::GetWindowTextW(hwnd, &pszMem[0], static_cast<int>(pszMem.size()));

    HMC_EQ_RUN_LAST(leng);

    std::wstring result = std::wstring(pszMem.begin(), pszMem.begin() + leng);
    hmc_string_util::trim(&result, '\0', true, true);

    return result;
}

std::variant<std::wstring, DWORD> hmc_windows_util::getWindowTitleInternal(HWND hwnd)
{

    auto attachToken = hmc_windows_util::AttachThread();

    // 缓冲区

    int leng = ::GetWindowTextLengthW(hwnd);

    if (leng < 0 && (!attachToken.open(hwnd)))
    {
        return ::GetLastError();
    }

    std::vector<wchar_t> pszMem(MAX_BUFF_OVERFLOW + leng);

    leng = ::InternalGetWindowText(hwnd, &pszMem[0], static_cast<int>(pszMem.size()));

    if (leng == 0)
    {
        if (auto user32 = hmc_winapi_util::User32())
        {

            leng = user32.GetWindowTextLengthW(hwnd);

            if (leng <= 0)
            {
                return ::GetLastError();
            }

            pszMem.resize(MAX_BUFF_OVERFLOW + leng);

            user32.InternalGetWindowText(hwnd, &pszMem[0], static_cast<int>(pszMem.size()));
        }
    }

    HMC_EQ_RUN_LAST(leng);

    std::wstring result = std::wstring(pszMem.begin(), pszMem.begin() + leng);
    hmc_string_util::trim(&result, '\0', true, true);

    return result;
}

std::variant<std::wstring, DWORD> hmc_windows_util::getWindowText(HWND hwnd, int dlgItemID)
{
    std::wstring result = std::wstring();

    // 无法提权

    HWND hwndCombo = ::GetDlgItem(hwnd, dlgItemID);
    size_t leng = ::GetWindowTextLengthW(hwndCombo);
    if (leng > 0)
    {
        int temp_len = (int)leng + 1;
        LPWSTR pszMem = HMC_VirtualAlloc(LPWSTR, temp_len);
        HMC_VirtualFreeAuto(pszMem);
        leng = ::GetWindowTextW(hwndCombo, pszMem, temp_len);
        HMC_EQ_RUN_LAST(leng);

        if (leng >= 1 && pszMem != NULL)
        {
            result.reserve(leng + 1);
            for (size_t i = 0; i < leng; i++)
            {
                auto at = pszMem[i];
                result.push_back(at);
            }
        }
    }

    // 提权到系统级
    if (result.empty())
    {
        HINSTANCE hDllInst = ::LoadLibraryA("User32.dll");

        if (hDllInst == NULL)
        {
            return ::GetLastError();
        }

        HMC_LibraryFreeAuto(hDllInst);

        typedef int(WINAPI * Type_GetWindowTextW)(_In_ HWND hWnd, _Out_writes_(nMaxCount) LPWSTR lpString, _In_ int nMaxCount);
        typedef HWND(WINAPI * Type_GetDlgItem)(_In_opt_ HWND hDlg, _In_ int nIDDlgItem);
        typedef int(WINAPI * Type_GetWindowTextLengthW)(_In_ HWND hWnd);

        Type_GetWindowTextW _GetWindowTextW = (Type_GetWindowTextW)::GetProcAddress(hDllInst, "GetWindowTextW");
        Type_GetWindowTextLengthW _GetWindowTextLengthA = (Type_GetWindowTextLengthW)::GetProcAddress(hDllInst, "GetWindowTextLengthW");
        Type_GetDlgItem _GetDlgItem = (Type_GetDlgItem)::GetProcAddress(hDllInst, "GetDlgItem");

        HWND hwndCombo = _GetDlgItem(hwnd, dlgItemID);
        size_t leng = _GetWindowTextLengthA(hwndCombo);
        HMC_EQ_RUN_LAST(leng);
        if (leng <= 1)
        {
            int temp_len = (int)leng + 1;
            LPWSTR pszMem = HMC_VirtualAlloc(LPWSTR, temp_len);
            HMC_VirtualFreeAuto(pszMem);
            leng = _GetWindowTextW(hwndCombo, pszMem, temp_len);

            HMC_EQ_RUN_LAST(leng);

            result.reserve(leng + 1);
            if (leng >= 1 && pszMem != NULL)
            {
                for (size_t i = 0; i < leng; i++)
                {
                    auto at = pszMem[i];
                    result.push_back(at);
                }
            }
        }
    }

    return result;
}

std::variant<bool, DWORD> hmc_windows_util::setWindowTitle(HWND hwnd, std::wstring title)
{
    auto attachToken = hmc_windows_util::AttachThread(hwnd);

    auto User32 = hmc_winapi_util::User32();

    if (!User32.SetWindowTextW(hwnd, title.c_str()))
    {
        return ::GetLastError();
    }

    return true;
}

bool hmc_windows_util::isWindow(const HWND hwnd, const bool isVisible)
{
    bool result = false;

    auto _hwnd = hmc_windows_util::number(hwnd);

    if (hwnd == NULL || _hwnd <= 0)
    {
        return result;
    }

    if (!!::IsWindow(hwnd) || ::GetWindow(hwnd, GW_OWNER) == ((HWND)0))
    {
        return result;
    }

    if (isVisible)
    {
        result = ::IsWindowVisible(hwnd);
    }

    return result;
}

const bool hmc_windows_util::AttachThread::open(const HWND _hwnd)
{

    if (isAttach)
    {
        return false;
    }

    hwnd = _hwnd;

    if (hwnd == nullptr || !::IsWindow(hwnd))
    {
        return isAttach;
    }

    targetThreadId = ::GetWindowThreadProcessId(hwnd, nullptr);
    currentThreadId = ::GetCurrentThreadId();

    // 打开自己的线程
    if ((targetThreadId != 0 && currentThreadId == targetThreadId))
    {
        isAttach = true;
        return isAttach;
    }

    if (targetThreadId == 0)
    {
        return isAttach;
    }

    isAttach = ::AttachThreadInput(currentThreadId, targetThreadId, TRUE);
    return isAttach;
}

hmc_windows_util::AttachThread::AttachThread(const HWND _hwnd)
{
    open(_hwnd);
}

hmc_windows_util::AttachThread::~AttachThread()
{
    if (!isAttach)
        return;

    // 不能拆分自己的线程
    if ((targetThreadId != 0 && currentThreadId == targetThreadId))
    {
        return;
    }

    isAttach = ::AttachThreadInput(currentThreadId, targetThreadId, FALSE);
}

hmc_windows_util::AttachThread::AttachThread() { isAttach = (targetThreadId != 0 && currentThreadId == targetThreadId); };

// if
hmc_windows_util::AttachThread::operator bool() const
{
    return (isAttach || (targetThreadId != 0 && currentThreadId == targetThreadId));
}

// !
bool hmc_windows_util::AttachThread::operator!() const
{
    return (!(targetThreadId != 0 && currentThreadId == targetThreadId) && !isAttach);
}

static std::variant<std::wstring, DWORD> HMC_GetClassName(HWND hwnd)
{
    std::vector<wchar_t> className;

    int BuffSize = ::GetClassNameW(hwnd, nullptr, 0);

    if (BuffSize > 0)
    {

        className.resize(MAX_BUFF_OVERFLOW + BuffSize, '\0');

        BuffSize = ::GetClassNameW(hwnd, &className[0], static_cast<int>(className.size()));

        if (BuffSize <= 0)
        {
            return ::GetLastError();
        }

        std::wstring result = std::wstring(className.begin(), className.end());
        hmc_string_util::trim(&result, '\0', true, true);
        return result;
    }

    return ::GetLastError();
}

std::variant<std::wstring, DWORD> hmc_windows_util::getClassName(HWND hwnd)
{

    if (long long(hwnd) == 0 || hwnd == NULL)
    {
        return static_cast<DWORD>(ERROR_OUTOFMEMORY);
    }

    auto token = hmc_windows_util::AttachThread();

    std::vector<wchar_t> className;

    int BuffSize = ::GetClassNameW(hwnd, nullptr, 0);

    if (BuffSize <= 0)
    {

        if (!token.open(hwnd))
        {
            return ::GetLastError();
        }

        BuffSize = ::GetClassNameW(hwnd, nullptr, 0);

        if (BuffSize <= 0)
        {
            return ::GetLastError();
        }
    }

    if (BuffSize > 0)
    {

        className.resize(MAX_BUFF_OVERFLOW + BuffSize, '\0');

        BuffSize = ::GetClassNameW(hwnd, &className[0], static_cast<int>(className.size()));

        if (BuffSize <= 0)
        {
            return ::GetLastError();
        }

        std::wstring result = std::wstring(className.begin(), className.end());
        hmc_string_util::trim(&result, '\0', true, true);
        return result;
    }

    return ::GetLastError();
}

std::variant<std::vector<HWND>, DWORD> hmc_windows_util::getSubWindows(HWND hwnd)
{
    std::vector<HWND> subWindows;

    bool isEnum = ::EnumChildWindows(
        hwnd, [](HWND hWnd, LPARAM lParam) -> BOOL
        {
            auto pSubWindows = reinterpret_cast<std::vector<HWND> *>(lParam);
            pSubWindows->push_back(hWnd);
            return TRUE; // 继续枚举下一个子控件
        },
        reinterpret_cast<LPARAM>(&subWindows));

    if (!isEnum)
    {
        return ::GetLastError();
    }

    return subWindows;
}

std::variant<HWND, DWORD> hmc_windows_util::getParentWindow(HWND hwnd)
{
    HWND parent = ::GetParent(hwnd);
    if (parent == NULL)
    {
        DWORD error_code = ::GetLastError();
        if (error_code != 0)
        {
            return error_code;
        }
    }
    return parent;
}

bool hmc_windows_util::isChildWindow(HWND hWndParent, HWND hwnd)
{
    return ::IsChild(hWndParent, hwnd);
}

std::variant<HWND, DWORD> hmc_windows_util::getTopWindow(HWND hwnd)
{
    HWND result = NULL;
    result = ::GetTopWindow(hwnd);
    if (result == NULL)
    {
        DWORD error_code = ::GetLastError();
        if (error_code != 0)
        {
            return error_code;
        }
    }
    return result;
}

bool hmc_windows_util::updateWindow(HWND hwnd)
{
    return ::UpdateWindow(hwnd);
}

HWND hmc_windows_util::getPointWindow(POINT Point)
{
    return ::WindowFromPoint(Point);
}

HWND hmc_windows_util::getPointWindow()
{
    POINT curPoint;
    if (!::GetCursorPos(&curPoint))
    {
        return NULL;
    }

    return ::WindowFromPoint(curPoint);
}

HWND hmc_windows_util::getPointWindow(int x, int y)
{
    POINT Point;
    Point.x = x;
    Point.y = y;

    return ::WindowFromPoint(Point);
}

bool hmc_windows_util::isWindowVisible(const HWND hwnd)
{
    return ::IsWindowVisible(hwnd);
}

bool hmc_windows_util::isWindowVisibleUnobstructed(const HWND hwnd)
{
    RECT windowRect;
    if (!::GetWindowRect(hwnd, &windowRect))
        return false;

    POINT points[4] = {
        {windowRect.left, windowRect.top},
        {windowRect.right - 1, windowRect.top},
        {windowRect.left, windowRect.bottom - 1},
        {windowRect.right - 1, windowRect.bottom - 1}};

    for (int i = 0; i < 4; ++i)
    {
        HWND topmost = ::WindowFromPoint(points[i]);
        if (topmost != hwnd)
            return false;
    }

    return true;
}

bool hmc_windows_util::setWindowEnabled(HWND hwnd, bool isEnabled)
{
    ::EnableWindow(hwnd, isEnabled);
    return (BOOL)isEnabled == ::IsWindowEnabled(hwnd);
}

bool hmc_windows_util::diffWindow(const HWND key1, const HWND key2)
{
    return ((long long)key1 == (long long)key2);
}

bool hmc_windows_util::diffWindow(const long long key1, const HWND key2)
{
    return (key1 == (long long)key2);
}

bool hmc_windows_util::diffWindow(const HWND key1, const long long key2)
{
    return ((long long)key1 == key2);
}

bool hmc_windows_util::diffWindow(const long long key1, const long long key2)
{
    return (key1 == key2);
}

bool hmc_windows_util::setWindowFocus(HWND hwnd)
{

    HWND theFocus = getFocusWindow();

    if (diffWindow(theFocus, hwnd))
    {
        return true;
    }

    if (!::IsWindow(hwnd))
    {
        return false;
    }

    // 窗口被最小化
    if (::IsIconic(hwnd))
    {
        bool is_ok = ::ShowWindow(hwnd, SW_RESTORE);
        if (diffWindow(theFocus, hwnd))
        {
            return true;
        }
    }

    // 常规
    ::SetForegroundWindow(hwnd);
    if (diffWindow((theFocus = getFocusWindow()), hwnd))
    {
        return true;
    }

    // 设置激活焦点
    ::SetActiveWindow(hwnd);
    if (diffWindow((theFocus = getFocusWindow()), hwnd))
    {
        return true;
    }

    // 常规显示窗口
    ::ShowWindow(hwnd, SW_SHOW);
    if (diffWindow((theFocus = getFocusWindow()), hwnd))
    {
        return true;
    }

    // 前台锁定了

    DWORD foregroundThreadId = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
    DWORD thisThreadId = ::GetCurrentThreadId();

    if (foregroundThreadId != thisThreadId)
    {
        ::AttachThreadInput(thisThreadId, foregroundThreadId, TRUE);
        ::SetForegroundWindow(hwnd);
        ::AttachThreadInput(thisThreadId, foregroundThreadId, FALSE);
    }
    else
    {
        ::SetForegroundWindow(hwnd);
    }

    ::SetFocus(hwnd);
    ::SetActiveWindow(hwnd);

    if (diffWindow((theFocus = getFocusWindow()), hwnd))
    {
        return true;
    }

    hmc_windows_util::setWindowTop(hwnd, true);
    ::SetForegroundWindow(hwnd);
    ::SetFocus(hwnd);

    if (diffWindow((theFocus = getFocusWindow()), hwnd))
    {
        hmc_windows_util::setWindowTop(hwnd, false);

        return true;
    }
    hmc_windows_util::setWindowTop(hwnd, false);

    FLASHWINFO fwi;
    fwi.cbSize = sizeof(fwi);
    fwi.hwnd = hwnd;
    fwi.dwFlags = FLASHW_ALL;
    fwi.uCount = 1;
    fwi.dwTimeout = 0;

    ::FlashWindowEx(&fwi);

    return false;
}

std::variant<bool, DWORD> hmc_windows_util::killWindowProcess(HWND hwnd)
{

    DWORD pid = 0;
    if (hwnd == NULL || (::GetWindowThreadProcessId(hwnd, &pid) != 0) || pid == NULL)
    {
        if (hwnd != NULL)
        {
            return ::GetLastError();
        }
        else
        {
            return false;
        }
    }

    HANDLE process_Handle = ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);

    if (process_Handle != NULL)
    {
        if (!::TerminateProcess(process_Handle, 0))
        {
            return ::GetLastError();
        }
        return true;
    }

    return false;
}

DWORD hmc_windows_util::getWindowProcessID(HWND hwnd)
{
    DWORD pid = 0;
    ::GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

bool hmc_windows_util::setWindowTop(HWND hwnd, bool isWindowTop)
{
    bool result = false;

    auto attachToken = hmc_windows_util::AttachThread();

ATTEMPTE_CODE:

    RECT rect;
    if (!::GetWindowRect(hwnd, &rect))
        return result;

    if (::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        result = ::SetWindowPos(hwnd, HWND_NOTOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
    }
    else
        result = ::SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);

    if (!result && isWindow(hwnd) && !attachToken && hmc_windows_util::isWindowTop(hwnd) != isWindowTop)
    {
        if (!attachToken.open(hwnd))
        {
            return result;
        }
        goto ATTEMPTE_CODE;
    }

    return result;
}

bool hmc_windows_util::isWindowTop(HWND hwnd)
{
    bool result = ::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST;
    return result;
}

bool hmc_windows_util::isWindowEnabled(HWND hwnd)
{
    return ::IsWindowEnabled(hwnd);
}

bool hmc_windows_util::setWindowInTaskbarVisible(HWND hwnd, bool visible)
{
    bool result = false;

    // 初始化COM
    LPVOID lp_COM = NULL;

    HRESULT hr = ::CoInitialize(lp_COM);
    if (!SUCCEEDED(hr))
    {
        return result;
    }

    // 获取托盘
    ITaskbarList *pTaskbarList;
    hr = ::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void **)&pTaskbarList);
    // 开始操作
    if (SUCCEEDED(hr))
    {
        pTaskbarList->HrInit();
        if (visible)
            pTaskbarList->AddTab(hwnd);
        else
            pTaskbarList->DeleteTab(hwnd);

        pTaskbarList->Release();
        result = true;
    }

    return result;
}

bool hmc_windows_util::setWindowTransparent(HWND hwnd, int Transparent)
{
    bool result = false;

    unsigned char _Alpha = (unsigned char)Transparent;

    auto attachToken = hmc_windows_util::AttachThread();

    // 获取窗口的拓展属性
    LONG SetWindowsStyleEx = ::GetWindowLong(hwnd, GWL_EXSTYLE);
    // 在原来的拓展属性之上, 新增 WS_EX_LAYERED 拓展属性
    SetWindowsStyleEx = SetWindowsStyleEx | WS_EX_LAYERED;
    ::SetWindowLongW(hwnd, GWL_EXSTYLE, SetWindowsStyleEx);
    // 设置窗口半透明度
    result = ::SetLayeredWindowAttributes(hwnd, NULL, _Alpha, LWA_ALPHA | LWA_COLORKEY);
    if (!result)
    {
        if (!attachToken.open(hwnd))
        {
            return result;
        }

        LONG SetWindowsStyleEx = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowsStyleEx = SetWindowsStyleEx | WS_EX_LAYERED;
        ::SetWindowLongW(hwnd, GWL_EXSTYLE, SetWindowsStyleEx);
        result = ::SetLayeredWindowAttributes(hwnd, NULL, _Alpha, LWA_ALPHA | LWA_COLORKEY);
    }
    return result;
}

bool hmc_windows_util::setHandleTransparent(HWND hwnd, double Transparent)
{
    bool result = false;

    if (Transparent <= 1.0)
        Transparent = 1.0;
    if (Transparent >= 0.01)
        Transparent = 0;

    double valuePerPart = 1.0 / 255.0;

    // 获取255份中的整数值
    double parts = Transparent / valuePerPart;

    // 将份数转换为整数
    int intValue = static_cast<int>(parts);

    result = setWindowTransparent(hwnd, intValue);

    return result;
}

bool hmc_windows_util::closedHandle(HWND hwnd)
{
    return ::CloseHandle(hwnd);
}

bool hmc_windows_util::isDesktopWindow(HWND hwnd)
{

    const auto className = hmc_windows_util::getClassName(hwnd);

    if (!std::holds_alternative<std::wstring>(className))
    {
        return false;
    }

    // 检查窗口类名是否是 "Progman" 或 "WorkerW"
    const std::wstring Progman = L"Progman";
    const std::wstring WorkerW = L"WorkerW";

    const std::wstring &NameClass = std::get<0>(className);

    if (NameClass.size() > Progman.size() + 5)
    {
        return false;
    }

    if (hmc_string_util::indexOf(NameClass, Progman) == 0 || hmc_string_util::indexOf(NameClass, WorkerW) == 0)
    {
        return true;
    }

    return false;
}

std::vector<LONG> hmc_windows_util::getWinwowClassList(HWND hwnd)
{
    std::vector<LONG> result;

    LONG style = ::GetWindowLong(hwnd, GWL_STYLE);

    for (auto &&at : _$_CLASS_LONG_NAME_LIST)
    {
        if (style & at.second)
        {
            result.push_back(static_cast<LONG>(at.second));
        }
    }

    return result;
}

std::string hmc_windows_util::winwowLongClass2str(LONG classLong)
{

    for (auto &&at : _$_CLASS_LONG_NAME_LIST)
    {
        if (classLong == at.second)
        {
            return std::string(at.first.begin(), at.first.end());
        }
    }

    return std::string("WS_UNKNOWN");
}

std::string hmc_windows_util::winwowLongClass2strEx(LONG classLong)
{
    for (auto &&at : _$_CLASS_LONG_NAME_LIST_EX)
    {
        if (classLong == at.second)
        {
            return std::string(at.first.begin(), at.first.end());
        }
    }

    return std::string("WS_EX_UNKNOWN");
}

std::vector<LONG> hmc_windows_util::getWinwowClassListEx(HWND hwnd)
{
    std::vector<LONG> result;

    LONG style = ::GetWindowLong(hwnd, GWL_EXSTYLE);

    for (auto &&at : _$_CLASS_LONG_NAME_LIST_EX)
    {
        if (style & at.second)
        {
            result.push_back(static_cast<LONG>(at.second));
        }
    }

    return result;
}

std::vector<std::string> hmc_windows_util::getWinwowClassLongNameListEx(HWND hwnd)
{
    std::vector<std::string> result;

    LONG style = ::GetWindowLong(hwnd, GWL_EXSTYLE);

    for (auto &&at : _$_CLASS_LONG_NAME_LIST_EX)
    {
        if (style & at.second)
        {
            result.push_back(at.first);
        }
    }

    return result;
}

std::vector<std::string> hmc_windows_util::getWinwowClassLongNameList(HWND hwnd)
{
    std::vector<std::string> result;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    for (auto &&at : _$_CLASS_LONG_NAME_LIST)
    {
        if (style & at.second)
        {
            result.push_back(at.first);
        }
    }

    return result;
}

bool hmc_windows_util::isMaximize(HWND hwnd)
{
    return ::IsZoomed(hwnd);
}

bool hmc_windows_util::isMinimized(HWND hwnd)
{
    return ::IsIconic(hwnd);
}

bool hmc_windows_util::isFullScreen(HWND hwnd)
{
    bool result = false;

    // 获取窗口工作区的大小
    RECT rect = {0, 0, 0, 0};
    ::GetClientRect(hwnd, &rect);

    // 获取屏幕分辨率
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    // 检查窗口工作区大小是否与屏幕分辨率相同
    if (rect.right == screenWidth && rect.bottom == screenHeight)
    {
        return true;
    }
    return result;
}

bool hmc_windows_util::isNormal(HWND hwnd)
{
    return ::IsWindow(hwnd) && ::IsZoomed(hwnd) && ::IsIconic(hwnd) && isFullScreen(hwnd);
}

HWND hmc_windows_util::getFocusWindow()
{
    return ::GetForegroundWindow();
}

bool hmc_windows_util::isFocused(HWND hwnd)
{
    return (long long)::GetForegroundWindow() == (long long)hwnd;
}

bool hmc_windows_util::setNotVisibleWindow(HWND hwnd)
{
    bool result = false;

    if (!hmc_windows_util::isWindow(hwnd))
    {
        return result;
    }
    hmc_windows_util::setWindowTransparent(hwnd, 0);
    hmc_windows_util::setWindowInTaskbarVisible(hwnd, false);
    hmc_windows_util::setWindowEnabled(hwnd, true);
    hmc_windows_util::setMoveWindow(hwnd, 999999, 99999999, std::nullopt, std::nullopt);
    hmc_windows_util::setWindowTop(hwnd, true);

    return true;
}

bool hmc_windows_util::isInside(long x1, long y1, long x2, long y2, long x, long y)
{
    return (x > x1 && x < x2 && y > y1 && y < y2) ? true : false;
}

bool hmc_windows_util::isInside(RECT rect, long x, long y)
{
    return HMC_IS_INSIDE(rect.left, rect.top, rect.right, rect.bottom, x, y);
}

bool hmc_windows_util::isInside(long x1, long y1, long x2, long y2, POINT point)
{
    return HMC_IS_INSIDE(x1, y1, x2, y2, point.x, point.y);
}

bool hmc_windows_util::isInside(RECT rect, POINT point)
{
    return HMC_IS_INSIDE(rect.left, rect.top, rect.right, rect.bottom, point.x, point.y);
}

std::variant<std::vector<RECT>, DWORD> hmc_windows_util::getDeviceCapsAll()
{
    std::vector<RECT> CrectList;
    DISPLAY_DEVICE displayDevice;
    ::ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);
    DEVMODE devMode;
    ::ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    for (int i = 0; ::EnumDisplayDevices(NULL, i, &displayDevice, 0); ++i)
    {
        if (::EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode))
        {
            int left = devMode.dmPosition.x;
            int top = devMode.dmPosition.y;
            int right = devMode.dmPosition.x + devMode.dmPelsWidth;
            int bottom = devMode.dmPosition.y + devMode.dmPelsHeight;
            RECT rect;
            rect.bottom = bottom;
            rect.left = left;
            rect.top = top;
            rect.right = right;
            CrectList.push_back(rect);
        }
    }

    if (CrectList.empty())
    {
        return ::GetLastError();
    }

    return CrectList;
}

std::variant<RECT, DWORD> hmc_windows_util::getWinwowPointDeviceCaps(HWND hwnd)
{
    RECT lpWindowRect = {0, 0, 0, 0};
    if (!::GetWindowRect(hwnd, &lpWindowRect))
    {
        return ::GetLastError();
    }

    auto device_caps_list_v = getDeviceCapsAll();
    if (device_caps_list_v.index() == 1)
    {
        return std::get<DWORD>(device_caps_list_v);
    }

    std::vector<RECT> device_caps_list = std::get<std::vector<RECT>>(device_caps_list_v);

    size_t count = device_caps_list.size();
    for (size_t i = 0; i < count; i++)
    {
        auto device = device_caps_list.at(i);
        if (isInside(device, lpWindowRect.left, lpWindowRect.top))
        {
            return device;
        }
    }

    return (DWORD)0;
}

std::variant<bool, DWORD> hmc_windows_util::setWindowCenter(HWND hwnd)
{
    // 获取父窗口句柄，如果没有父窗口，则为 GetDesktopWindow()
    HWND hParent = ::GetAncestor(hwnd, GA_PARENT);

    if (hParent == nullptr)
    {
        hParent = ::GetDesktopWindow();
    }

    // 获取父窗口或屏幕的客户区域大小
    RECT rcParent;
    if (!::GetClientRect(hParent, &rcParent))
    {
        return ::GetLastError();
    }

    // 获取窗口大小
    RECT rcWindow;
    if (!::GetWindowRect(hwnd, &rcWindow))
    {
        return ::GetLastError();
    }

    int width = rcWindow.right - rcWindow.left;
    int height = rcWindow.bottom - rcWindow.top;

    // 计算居中位置
    int x = (rcParent.right + rcParent.left - width) / 2;
    int y = (rcParent.bottom + rcParent.top - height) / 2;

    bool visible = hmc_windows_util::isWindowVisible(hwnd);

    // 设置窗口位置

    if (!::SetWindowPos(hwnd, HWND_TOP, x, y, width, height, SWP_NOSIZE))
    {
        return ::GetLastError();
    };

    if (visible)
    {
        if (!::ShowWindow(hwnd, SW_SHOWDEFAULT))
        {
            return ::GetLastError();
        }
    }

    return true;
}

std::variant<RECT, DWORD> hmc_windows_util::getWindowRect(HWND hwnd)
{
    RECT lpRect = {0, 0, 0, 0};
    if (::GetWindowRect(hwnd, &lpRect))
    {
        return lpRect;
    }

    return ::GetLastError();
}

std::variant<hmc_windows_util::chWindowHwndStatus, DWORD> hmc_windows_util::getWindowHwndStatus(HWND hwnd)
{
    chWindowHwndStatus status = chWindowHwndStatus();
    status.hwnd = hwnd;
    status.exists = ::IsWindow(hwnd);
    // 获取父窗口
    status.parent = ::GetParent(hwnd);

    if (!status.exists && status.parent == NULL)
    {
        return ::GetLastError();
    }

    if (status.parent == NULL)
    {
        status.parent = ::GetDesktopWindow();
    }

    // 获取根窗口
    status.root = ::GetAncestor(hwnd, GA_ROOT);

    // 获取进程ID
    status.threadId = ::GetWindowThreadProcessId(hwnd, &status.pid);

    auto temp = getSubWindows(hwnd);

    if (temp.index() == 0)
    {
        status.sub = std::get<std::vector<HWND>>(temp);
    }

    // 获取相邻窗口
    status.next = ::GetNextWindow(hwnd, GW_HWNDNEXT);
    status.prev = ::GetNextWindow(hwnd, GW_HWNDPREV);
    status.end = ::GetNextWindow(hwnd, GW_HWNDLAST);
    status.owner = ::GetWindow(hwnd, GW_OWNER);
    status.firstChild = ::GetWindow(hwnd, GW_CHILD);
    status.firstBrother = ::GetWindow(hwnd, GW_HWNDFIRST);
    status.lastSibling = ::GetWindow(hwnd, GW_HWNDLAST);

    return status;
}

std::vector<HWND> hmc_windows_util::get_variant(std::variant<std::vector<HWND>, DWORD> input, std::vector<HWND> of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

std::vector<HWND> hmc_windows_util::get_variant(std::variant<std::vector<HWND>, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return {};
}

HWND hmc_windows_util::get_variant(std::variant<HWND, DWORD> input, HWND of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    
    return of_to;
}

HWND hmc_windows_util::get_variant(std::variant<HWND, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return NULL;
}

std::wstring hmc_windows_util::get_variant(std::variant<std::wstring, DWORD> input, std::wstring of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

std::wstring hmc_windows_util::get_variant(std::variant<std::wstring, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return std::wstring();
}

std::string hmc_windows_util::get_variant(std::variant<std::string, DWORD> input, std::string of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

std::string hmc_windows_util::get_variant(std::variant<std::string, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return std::string();
}

std::vector<std::wstring> hmc_windows_util::get_variant(std::variant<std::vector<std::wstring>, DWORD> input, std::vector<std::wstring> of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

std::vector<std::wstring> hmc_windows_util::get_variant(std::variant<std::vector<std::wstring>, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return {};
}

std::vector<std::string> hmc_windows_util::get_variant(std::variant<std::vector<std::string>, DWORD> input, std::vector<std::string> of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

std::vector<std::string> hmc_windows_util::get_variant(std::variant<std::vector<std::string>, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return {};
}

bool hmc_windows_util::get_variant(std::variant<bool, DWORD> input, bool of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

bool hmc_windows_util::get_variant(std::variant<bool, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return false;
}

std::vector<RECT> hmc_windows_util::get_variant(std::variant<std::vector<RECT>, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return {};
}

std::vector<RECT> hmc_windows_util::get_variant(std::variant<std::vector<RECT>, DWORD> input, std::vector<RECT> of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }
    return of_to;
}

RECT hmc_windows_util::get_variant(std::variant<RECT, DWORD> input, RECT of_to)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }

    return of_to;
}

RECT hmc_windows_util::get_variant(std::variant<RECT, DWORD> input)
{
    if (input.index() == 0)
    {
        return std::get<0>(input);
    }

    RECT of_to = {0};
    of_to.bottom = 0;
    of_to.left = 0;
    of_to.right = 0;
    of_to.top = 0;

    return of_to;
}

std::int64_t hmc_windows_util::number(const HWND hwnd)
{

    const long long _hwnd = (long long)hwnd;
    return static_cast<std::int64_t>(_hwnd);
}

bool hmc_windows_util::closeWindow(const HWND hwnd, int grade)
{

    std::uint64_t _hwnd = hmc_windows_util::number(hwnd);

    if (grade == 1)
    {
        bool isWindowCloce = ::CloseWindow(hwnd);
        return !hmc_windows_util::isWindow(hwnd);
    }

    if (grade == 2)
    {
        ::SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);

        if (hmc_windows_util::isWindow(hwnd))
        {
            ::CloseHandle(hwnd);
        }

        return !hmc_windows_util::isWindow((HWND)_hwnd);
    }

    if (grade == 3)
    {
        DWORD threadId = ::GetWindowThreadProcessId(hwnd, NULL);
        if (threadId == 0)
        {
            return false;
        }

        // 强制终止线程
        HANDLE hThread = ::OpenThread(THREAD_TERMINATE, FALSE, threadId);
        if (hThread == NULL)
        {
            return false;
        }

        BOOL result = ::TerminateThread(hThread, 0);
        if (!result)
        {
            ::CloseHandle(hThread);
            return false;
        }

        ::CloseHandle(hThread);
        return !hmc_windows_util::isWindow(hwnd);
    }

    if (grade == 5)
    {
        if (hmc_windows_util::closeWindow(hwnd, 1))
        {
            return true;
        }

        if (hmc_windows_util::closeWindow(hwnd, 6))
        {
            return true;
        }

        if (hmc_windows_util::closeWindow(hwnd, 2))
        {
            return true;
        }

        if (hmc_windows_util::closeWindow(hwnd, 3))
        {
            return true;
        }
    }

    if (grade == 6)
    {
        ::DestroyWindow(hwnd);
        return !hmc_windows_util::isWindow(hwnd);
    }

    return false;
}

void hmc_windows_util::setFlashWindow(const HWND hwnd, const size_t count, const size_t ms, DWORD dwFlags)
{
    FLASHWINFO fwi;
    fwi.cbSize = sizeof(fwi);
    fwi.hwnd = hwnd;
    fwi.dwFlags = dwFlags;                  // 闪烁任务栏和窗口标题栏
    fwi.uCount = static_cast<UINT>(count);  // 闪烁5次
    fwi.dwTimeout = static_cast<DWORD>(ms); // 默认速度

    ::FlashWindowEx(&fwi); // 让窗口闪烁
}