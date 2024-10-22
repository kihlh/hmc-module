#include "hmc_screen_util.h"
#include "hmc_util.h"
#include "hmc_tray_util.h"
#include <unordered_map>
#include "hmc_process_util.h"
#include <shared_mutex>
#include <atomic>

std::vector<hmc_tray_util *> HMC_TRAY_UTIL_TRAY_LIST;
std::shared_mutex GpSharedMutex;
std::atomic<size_t> GpTaryID = 1;

#define WM_SYSICON (WM_USER + 1)

void hmc_tray_util::chMenuItem::__initStore()
{
    messageID = -1;
    select = false;
    menuList = {};
    disable = false;
    rootMenu = true;
    show = true;
    name = L"unknown";
    id = L"unknown" + std::to_wstring(messageID);
    type = chMenuType::menu;
};

hmc_tray_util::chMenuItem hmc_tray_util::chMenuItem::menu(std::wstring name, std::wstring id, bool disable)
{
    hmc_tray_util::chMenuItem menuItem;
    menuItem.disable = disable;
    menuItem.id = id;
    menuItem.name = name;
    menuItem.rootMenu = true;
    menuItem.menuList = {};
    menuItem.show = true;
    menuItem.select = false;
    menuItem.type = hmc_tray_util::chMenuType::menu;
    return menuItem;
}

hmc_tray_util::chMenuItem hmc_tray_util::chMenuItem::check(std::wstring name, std::wstring id, bool select)
{
    hmc_tray_util::chMenuItem menuItem;
    menuItem.disable = false;
    menuItem.id = id;
    menuItem.name = name;
    menuItem.rootMenu = true;
    menuItem.menuList = {};
    menuItem.show = true;
    menuItem.select = select;
    menuItem.type = hmc_tray_util::chMenuType::check;
    return menuItem;
}

hmc_tray_util::chMenuItem hmc_tray_util::chMenuItem::separator(std::wstring id, bool root)
{
    hmc_tray_util::chMenuItem menuItem;
    menuItem.disable = false;
    menuItem.id = id;
    menuItem.show = true;
    menuItem.rootMenu = root;
    menuItem.menuList = {};
    menuItem.select = false;
    menuItem.type = hmc_tray_util::chMenuType::separator;
    return menuItem;
}

static void $HMC_EMIT_ENVINFO(const std::wstring assert_IF, const std::wstring assert_IF02, hmc_tray_util::CALLBACK_ON_EVENT *ON_EVENT, hmc_tray_util::CALLBACK_ON_EVENT *ONCE_EVENT)
{
    for (auto &&onEvent : *ON_EVENT)
    {
        if (onEvent.first == assert_IF || onEvent.first == assert_IF02)
        {
            for (size_t i = 0; i < onEvent.second.size(); i++)
            {
                auto &fun = onEvent.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
        }
    }

    for (auto &&onEvent : *ONCE_EVENT)
    {
        if (onEvent.first == assert_IF || onEvent.first == assert_IF02)
        {
            for (size_t i = 0; i < onEvent.second.size(); i++)
            {
                auto &fun = onEvent.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
            ONCE_EVENT->clear();
        }
    };
}

static void $HMC_EMIT_ENVINFO(const std::wstring assert_IF, hmc_tray_util::CALLBACK_ON_EVENT *ON_EVENT, hmc_tray_util::CALLBACK_ON_EVENT *ONCE_EVENT)
{
    for (auto &&onEvent : *ON_EVENT)
    {
        if (onEvent.first == assert_IF)
        {
            for (size_t i = 0; i < onEvent.second.size(); i++)
            {
                auto &fun = onEvent.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
        }
    }

    for (auto &&onEvent : *ONCE_EVENT)
    {
        if (onEvent.first == assert_IF)
        {
            for (size_t i = 0; i < onEvent.second.size(); i++)
            {
                auto &fun = onEvent.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
            ONCE_EVENT->clear();
        }
    };
}

void hmc_tray_util::emit(LPARAM lParam)
{
    switch (lParam)
    {
        // 鼠标右键按下
    case WM_RBUTTONDOWN:
    {

        $HMC_EMIT_ENVINFO(L"rightButtonDown", &ON_EVENT, &ONCE_EVENT);

        break;
    }
        // 鼠标右键松开
    case WM_RBUTTONUP:
    {
        $HMC_EMIT_ENVINFO(L"rightButtonUp", &ON_EVENT, &ONCE_EVENT);
        break;
    }
        // 鼠标左键按下
    case WM_LBUTTONDOWN:
    {
        $HMC_EMIT_ENVINFO(L"leftButtonDown", &ON_EVENT, &ONCE_EVENT);

        break;
    }
        // 鼠标左键松开
    case WM_LBUTTONUP:
    {
        $HMC_EMIT_ENVINFO(L"click", L"leftButtonUp", &ON_EVENT, &ONCE_EVENT);

        break;
    }
        // 鼠标左键双击
    case WM_LBUTTONDBLCLK:
    {
        $HMC_EMIT_ENVINFO(L"dblclick", L"leftButtonDoubleClick", &ON_EVENT, &ONCE_EVENT);
        break;
    }
        // 鼠标浮动
    case WM_MOUSEMOVE:
    {
        $HMC_EMIT_ENVINFO(L"move", L"mouseMove", &ON_EVENT, &ONCE_EVENT);
        break;
    }
    // 中键点击
    case WM_MBUTTONDOWN:
    {
        $HMC_EMIT_ENVINFO(L"middleClick", &ON_EVENT, &ONCE_EVENT);
        break;
    }
    }
}

void hmc_tray_util::emit(std::wstring keyID, hmc_tray_util::chMenuItem &menuItem)
{
    bool isOnOK = false;
    for (auto &&event : ON_EVENT)
    {

        if (event.first == keyID)
        {
            for (size_t i = 0; i < event.second.size(); i++)
            {
                auto &fun = event.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
            isOnOK = true;
        }
    }

    bool isOnCeOK = false;
    for (auto &&event : ONCE_EVENT)
    {
        if (event.first == keyID)
        {
            for (size_t i = 0; i < event.second.size(); i++)
            {
                auto &fun = event.second[i];
                if (fun != nullptr)
                {
                    fun();
                };
            }
            isOnCeOK = true;
            event.second.clear();
        }
    }
}

void hmc_tray_util::clickButtonItem(WORD IDKey)
{
    std::wstring id = std::wstring();
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.messageID == IDKey)
        {
            id.clear();
            id.append(Menu.id);
            if (Menu.type == hmc_tray_util::chMenuType::check)
            {
                m_MenuList[i].select = !Menu.select;
            }
            emit(id, m_MenuList[i]);
        }
    }
}

/**
 * @brief 监听
 *
 * @param eventName
 * @param fnc
 */
void hmc_tray_util::on(std::wstring eventName, std::function<void()> fnc)
{

    std::vector<std::function<void()>> funList = {};

    if (ON_EVENT.find(eventName) == ON_EVENT.end())
    {
        ON_EVENT.insert(std::make_pair(eventName, funList));
    }

    for (auto &&EVENT : ON_EVENT)
    {
        if (EVENT.first == eventName)
        {
            EVENT.second.push_back(fnc);
        }
    }
}

/**
 * @brief 单次监听
 *
 * @param eventName
 * @param fnc
 */
void hmc_tray_util::once(std::wstring eventName, std::function<void()> fnc)
{
    std::vector<std::function<void()>> funList = {};

    if (ONCE_EVENT.find(eventName) == ONCE_EVENT.end())
    {
        ONCE_EVENT.insert(std::make_pair(eventName, funList));
    }

    for (auto &&EVENT : ONCE_EVENT)
    {
        if (EVENT.first == eventName)
        {
            EVENT.second.push_back(fnc);
        }
    }
}

bool hmc_tray_util::setTrayIcon(HICON hNewIcon, int index)
{
    try
    {
        nid.hIcon = hNewIcon;
        return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
    HMC_CATCH_ALL;

    return false;
}

bool hmc_tray_util::setTrayIcon(std::wstring Icons, int index)
{
    try
    {
        HICON hNewIcon = NULL; // 声明一个HICON句柄
        hNewIcon = ::ExtractIconW(NULL, Icons.c_str(), (UINT)index);
        if (hNewIcon != NULL)
        {
            nid.hIcon = hNewIcon;
            return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
    }
    HMC_CATCH_ALL;

    return false;
}

/**
 * @brief 修改tray的图标（从当前可执行文件获取）
 *
 * @return BOOL
 */
bool hmc_tray_util::setTrayIcon(int index)
{
    try
    {
        std::wstring the_path = hmc_process_util::getProcessFilePath();

        HICON hNewIcon = ::ExtractIconW(NULL, the_path.c_str(), index);

        if (hNewIcon == nullptr)
        {
            return false;
        }

        nid.hIcon = hNewIcon;
        return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
    HMC_CATCH_ALL;
    return false;
}

void hmc_tray_util::GC_ALL()
{
    if(HMC_TRAY_UTIL_TRAY_LIST.empty()){
        return;
    }
    
    for (auto &&at : HMC_TRAY_UTIL_TRAY_LIST)
    {
       if(at!=nullptr) at->close();
    }
}

/**
 * @brief 启动
 *
 */
bool hmc_tray_util::start()
{

    if (m_StartTray)
        return false;

    m_id = GpTaryID.fetch_add(1, std::memory_order_seq_cst);

    std::unique_lock<std::shared_mutex> writer_mutex(GpSharedMutex);
    HMC_TRAY_UTIL_TRAY_LIST.push_back(this);
    writer_mutex.unlock();

    m_TrayWorkerThread = new std::thread([this]
                                         {
                                        this->m_ThreadId = ::GetCurrentThreadId();
                                        this->c_openPuppetTrayWindow(); });

    m_ThreadId = hmc_util::toThreadId(m_TrayWorkerThread->get_id());

    m_StartTray = m_TrayWorkerThread != nullptr;

    if (m_id == 1)
    {
        ::atexit(hmc_tray_util::GC_ALL);
    }

    return m_StartTray;
}

// 结束托盘
bool hmc_tray_util::close()
{
    if (this != nullptr && ::CloseWindow(m_Hwnd))
    {
        try
        {
            ::PostThreadMessage(::GetThreadId(m_TrayWorkerThread->native_handle()), WM_QUIT, NULL, NULL);
            m_TrayWorkerThread->join();
            delete m_TrayWorkerThread;
            m_TrayWorkerThread = nullptr;
        }
        HMC_CATCH_ALL;

        try
        {
            m_StartTray = false;
            m_Hwnd = NULL;
            m_MenuList.clear();

            // 清空监听
            for (auto &&EVENT : ON_EVENT)
                EVENT.second.clear();
            for (auto &&EVENT : ONCE_EVENT)
                EVENT.second.clear();
            ON_EVENT.clear();
            ONCE_EVENT.clear();

            ::Shell_NotifyIconW(NIM_DELETE, &nid);
            //::ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
            std::unique_lock<std::shared_mutex> writer_mutex(GpSharedMutex);
            for (size_t i = 0; i < HMC_TRAY_UTIL_TRAY_LIST.size(); i++)
            {
                auto &at = HMC_TRAY_UTIL_TRAY_LIST.at(i);
                if (at == nullptr || at->id() == this->m_id)
                {
                    HMC_TRAY_UTIL_TRAY_LIST.erase(HMC_TRAY_UTIL_TRAY_LIST.begin() + i);
                    break;
                }
            }
            writer_mutex.unlock();
            delete this;
        }
        HMC_CATCH_ALL;
        return true;
    }
    return false;
}

/**
 * @brief 添加按钮
 *
 * @param menuItem
 * @return true
 * @return false
 */
bool hmc_tray_util::addMenuItem(hmc_tray_util::chMenuItem menuItem)
{
    for (auto &&Menu : m_MenuList)
    {
        if (Menu.id == menuItem.id)
            return false;
    }

    int idkey = static_cast<int>(m_TrayNextId++);

    menuItem.messageID = idkey;

    m_MenuList.push_back(menuItem);

    return true;
}

// 修改托盘显示的消息内容
bool hmc_tray_util::setTrayInfo(std::wstring trayInfo)
{
    std::wstring subtxt = std::wstring();
    try
    {
        if (trayInfo.length() > 255)
        {
            subtxt.append(trayInfo.substr(0, 255));
        }
        else
        {
            subtxt.append(trayInfo.substr(0, trayInfo.length()));
        }

        ::wcsncpy_s(nid.szTip, subtxt.c_str(), subtxt.size());

        if (::Shell_NotifyIconW(NIM_MODIFY, &nid))
        {
            m_TrayInfo.clear();
            m_TrayInfo.append(subtxt);
            return true;
        };
    }
    HMC_CATCH_ALL;
    return false;

    // wcscpy_s(nid.szTip, pszNewInfo);
}

LRESULT CALLBACK hmc_tray_util::c_CallBackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    auto theHwnd = (int64_t)(hWnd);

    if (theHwnd != 0)
    {
        std::shared_lock<std::shared_mutex> read_mutex(GpSharedMutex);

        if (HMC_TRAY_UTIL_TRAY_LIST.empty())
        {
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }
        read_mutex.unlock();
    }

    auto targetThreadId = hWnd == NULL ? 0 : (::GetWindowThreadProcessId(hWnd, nullptr));
    auto currentThreadId = ::GetCurrentThreadId();

    // 环境意外丢失

    hmc_tray_util *trayPtr = nullptr;

    std::shared_lock<std::shared_mutex> read_mutex(GpSharedMutex);

    for (size_t i = 0; i < HMC_TRAY_UTIL_TRAY_LIST.size(); i++)
    {
        auto &at = HMC_TRAY_UTIL_TRAY_LIST.at(i);

        auto at_hwnd = int64_t(at->m_Hwnd);

        if ((at_hwnd > 0 && at_hwnd == theHwnd))
        {
            trayPtr = HMC_TRAY_UTIL_TRAY_LIST.at(i);
            continue;
        }

        if (at->m_ThreadId == currentThreadId || at->m_ThreadId == targetThreadId)
        {
            at->m_ThreadId = targetThreadId;
            trayPtr = HMC_TRAY_UTIL_TRAY_LIST.at(i);
            trayPtr->m_hwndID = int64_t(hWnd);
            trayPtr->m_Hwnd = hWnd;
        }
    }
    read_mutex.unlock();

    switch (message)
    {
    case WM_TIMER:
    {

        if (wParam == 1)
        {
        }

        break;
    }
    case WM_CREATE:

        if (trayPtr != nullptr)
        {

            trayPtr->nid.cbSize = sizeof(NOTIFYICONDATAW);
            trayPtr->nid.hWnd = hWnd;
            trayPtr->nid.uID;
            trayPtr->nid.uVersion = NOTIFYICON_VERSION;
            trayPtr->nid.uCallbackMessage = WM_SYSICON;
            if (!trayPtr->nid.hIcon)
            {
                trayPtr->nid.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
            }

            ::wcsncpy_s(trayPtr->nid.szTip, trayPtr->m_Title.c_str(), trayPtr->m_Title.size());

            trayPtr->nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

            ::Shell_NotifyIconW(NIM_ADD, &((*trayPtr).nid));
        }

        break;

    case WM_DESTROY:

        if (trayPtr != nullptr)
        {
            ::Shell_NotifyIconW(NIM_DELETE, &((*trayPtr).nid));
        }
        ::KillTimer(hWnd, 1);
        ::PostQuitMessage(0);
        break;

    case WM_SYSICON:
    {

        if (trayPtr != nullptr)
        {
            trayPtr->emit(lParam);
        }

        // 如果是右键显示右键菜单
        if (trayPtr != nullptr && lParam == WM_RBUTTONUP)
        {
            POINT curPoint;
            ::GetCursorPos(&curPoint);
            ::SetForegroundWindow(hWnd);
            HMENU hMenu = ::CreatePopupMenu();

            HMC_FreeAnyAuto({
                if (hMenu == nullptr)
                    return;
                try
                {
                    ::DestroyMenu(hMenu);
                    hMenu = NULL;
                }
                HMC_CATCH_ALL;
            });

            if (hMenu)
            {
                for (auto &&Menu : trayPtr->m_MenuList)
                {

                    // 根按钮  且为按钮按钮中无子分类 分割线和选项将忽略子按钮检测
                    if (Menu.rootMenu && Menu.menuList.size() == 0)
                    {
                        if (!Menu.show)
                            continue;
                        // 按钮
                        if (Menu.type == hmc_tray_util::chMenuType::menu)
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYPOSITION, Menu.messageID, Menu.name.c_str());
                        }
                        // 分割线
                        else if (Menu.type == hmc_tray_util::chMenuType::separator)
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, Menu.messageID, Menu.name.c_str());
                        }
                        // 选项
                        else if (Menu.type == hmc_tray_util::chMenuType::check)
                        {
                            if (Menu.select)
                            {
                                ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MF_CHECKED, Menu.messageID, Menu.name.c_str());
                            }
                            else
                            {
                                ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MF_UNCHECKED, Menu.messageID, Menu.name.c_str());
                            }
                        }
                        // 选项
                        // else if (Menu.type == hmc_tray_util::chMenuType::radio)
                        // {
                        //     if (Menu.select)
                        //     {
                        //         ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MFT_RADIOCHECK | MF_CHECKED, Menu.messageID, Menu.name.c_str());
                        //     }
                        //     else
                        //     {
                        //         ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MFT_RADIOCHECK | MF_UNCHECKED, Menu.messageID, Menu.name.c_str());
                        //     }
                        // }

                        // 禁用
                        if (Menu.disable)
                        {
                            ::EnableMenuItem(hMenu, Menu.messageID, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                        }
                    }
                    // 有子目录的按钮
                    else if (Menu.show && Menu.rootMenu && Menu.menuList.size() != 0)
                    {
                        ::InsertMenuW(hMenu, -1, MF_BYPOSITION, Menu.messageID, Menu.name.c_str());
                        HMENU hSubMenu = ::CreatePopupMenu();
                        std::shared_ptr<void> close_hSubMenu(nullptr, [&](void *)
                                                             {if (hSubMenu != nullptr) {try{::DestroyMenu(hSubMenu);}HMC_CATCH_ALL;} });
                        if (!Menu.show)
                            continue;

                        for (auto &&IDmessageID : Menu.menuList)
                        {
                            // 创建子菜单
                            if (hSubMenu)
                            {
                                for (auto &&_Menu : trayPtr->m_MenuList)
                                {
                                    if (_Menu.messageID != IDmessageID)
                                        continue;

                                    // 按钮
                                    if (_Menu.type == hmc_tray_util::chMenuType::menu)
                                    {
                                        ::AppendMenuW(hSubMenu, MF_STRING, _Menu.messageID, _Menu.name.c_str());

                                        // ::InsertMenuW(hMenu, -1, MF_BYPOSITION, _Menu.messageID, _Menu.name.c_str());
                                    }
                                    // 分割线
                                    else if (_Menu.type == hmc_tray_util::chMenuType::separator)
                                    {
                                        ::AppendMenuW(hSubMenu, MF_SEPARATOR, _Menu.messageID, _Menu.name.c_str());
                                        // ::InsertMenuW(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, _Menu.messageID, _Menu.name.c_str());
                                    }
                                    // 选项
                                    else if (_Menu.type == hmc_tray_util::chMenuType::check)
                                    {
                                        if (_Menu.select)
                                        {
                                            ::AppendMenuW(hSubMenu, MF_CHECKED, _Menu.messageID, _Menu.name.c_str());
                                        }
                                        else
                                        {
                                            ::AppendMenuW(hSubMenu, MF_UNCHECKED, _Menu.messageID, _Menu.name.c_str());
                                        }
                                    }

                                    // 禁用
                                    if (_Menu.disable)
                                    {
                                        ::EnableMenuItem(hSubMenu, _Menu.messageID, MF_DISABLED | MF_GRAYED);
                                    }
                                }

                                // 把子菜单加到主菜单项
                                ::ModifyMenuW(hMenu, Menu.messageID, MF_POPUP, (UINT_PTR)hSubMenu, Menu.name.c_str());
                            }
                        }

                        // 这让菜单显示为需要的样子，例如处理具有复选菜单项的菜单
                        ::SetMenuDefaultItem(hMenu, Menu.messageID, FALSE);
                    }
                }
                ::TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, curPoint.x, curPoint.y, 0, hWnd, NULL);
                ::DestroyMenu(hMenu);
            }
        }
    }
    break;

    case WM_COMMAND:
        if (trayPtr)
            trayPtr->clickButtonItem(LOWORD(wParam));
    default:
        if (hWnd != NULL)
        {
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return 0;
}

int64_t hmc_tray_util::c_openPuppetTrayWindow()
{
    WNDCLASSEXW wc{};
    MSG Msg;
    HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style;

    wc.lpfnWndProc = c_CallBackWndProc;
    wc.cbClsExtra;
    wc.cbWndExtra;
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_ClassName.c_str();
    wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

    ::RegisterClassExW(&wc);

    m_Hwnd = ::CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, m_Title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, NULL, NULL, hInstance, NULL);

    if (m_Hwnd == NULL)
    {
        return ::GetLastError();
    }

    // 防呆
    ::SetTimer((const HWND)m_Hwnd, 1, 1000 * 30, NULL);

    ::ShowWindow((const HWND)m_Hwnd, SW_HIDE);
    // ::ShowWindow(hWnd, SW_SHOW);

    while (::GetMessageW(&Msg, NULL, 0, 0) > 0)
    {
        ::TranslateMessage(&Msg);
        ::DispatchMessage(&Msg);
    }

    return Msg.wParam;
}

/**
 * @brief 设置指定的按钮为XXX的子按钮
 *
 * @param MenuId
 * @param RootMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmetoSubMenu(std::wstring RootMenuId, std::wstring SubMenuId)
{
    for (auto &&menu : m_MenuList)
    {
        if (menu.id == RootMenuId /**|| menu.id == SubMenuId */)
        {
            menu.rootMenu = true;
            for (auto &&menu2 : m_MenuList)
            {
                if (menu2.id == SubMenuId /**|| menu2.id == SubMenuId */)
                {
                    menu2.rootMenu = false;
                    menu.menuList.push_back(menu2.messageID);
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * @brief 设置指定的按钮为XXX的子按钮
 *
 * @param RootMenuId
 * @param SubMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmetoSubMenu(chMenuItem RootMenuId, std::wstring SubMenuId)
{
    return setMenuItmetoSubMenu(RootMenuId.id, SubMenuId);
}

/**
 * @brief 设置指定的按钮为XXX的子按钮
 *
 * @param RootMenuId
 * @param SubMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmetoSubMenu(chMenuItem RootMenuId, chMenuItem SubMenuId)
{
    return setMenuItmetoSubMenu(RootMenuId.id, SubMenuId.id);
}

/**
 * @brief 设置指定的按钮为XXX的子按钮
 *
 * @param RootMenuId
 * @param SubMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmetoSubMenu(std::wstring RootMenuId, chMenuItem SubMenuId)
{
    return setMenuItmetoSubMenu(RootMenuId, SubMenuId.id);
}

bool hmc_tray_util::setMenuItmetoSubMenu(const std::wstring RootMenuId, const std::vector<std::wstring> SubMenuIdArgs)
{
    for (size_t i = 0; i < SubMenuIdArgs.size(); i++)
    {
        auto &at = SubMenuIdArgs.at(i);

        setMenuItmetoSubMenu(RootMenuId, at);
    }
    return true;
}

bool hmc_tray_util::setMenuItmetoSubMenu(chMenuItem RootMenuId, const std::vector<std::wstring> SubMenuIdArgs)
{
    for (size_t i = 0; i < SubMenuIdArgs.size(); i++)
    {
        auto &at = SubMenuIdArgs.at(i);

        setMenuItmetoSubMenu(RootMenuId, at);
    }
    return true;
}

/**
 * @brief 设置指定按钮可见性
 *
 * @param MenuId
 * @param Visible
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmeVisible(std::wstring MenuId, bool Visible)
{
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.id == MenuId)
        {

            m_MenuList[i].show = Visible ? true : false;
            return true;
        }
    }
    return false;
}

// 设置按钮为禁用
bool hmc_tray_util::setMenuItmeEnable(std::wstring MenuId, bool Enable)
{
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto Menu = m_MenuList[i];
        if (Menu.id == MenuId)
        {

            m_MenuList[i].disable = Enable;
            return true;
        }
    }
    return false;
}

/**
 * @brief 设置指定的按钮为XXX的子按钮
 *
 * @param RootMenuId
 * @param SubMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmeName(std::wstring MenuId, std::wstring Name)
{
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.id == MenuId)
        {

            m_MenuList[i].name.clear();
            m_MenuList[i].name.append(Name);
            return true;
        }
    }
    return false;
}

/**
 * @brief 设置指定的按钮为XXX的选择
 *
 * @param RootMenuId
 * @param SubMenuId
 * @return true
 * @return false
 */
bool hmc_tray_util::setMenuItmeSelect(std::wstring MenuId, bool Select)
{
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.id == MenuId)
        {
            m_MenuList[i].select = Select;
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the Menu Itme object
 *
 * @param MenuId
 * @return chMenuItem
 */
hmc_tray_util::chMenuItem hmc_tray_util::getMenuItme(std::wstring MenuId)
{

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.id == MenuId)
        {
            return m_MenuList[i];
        }
    }

    return chMenuItem();
}

const size_t hmc_tray_util::id()
{
    return size_t(0) + m_id;
}

hmc_tray_util *hmc_tray_util::at(size_t id)
{
    hmc_tray_util *result = nullptr;

    std::shared_lock<std::shared_mutex> read_mutex(GpSharedMutex);

    for (size_t i = 0; i < HMC_TRAY_UTIL_TRAY_LIST.size(); i++)
    {
        hmc_tray_util *atv = HMC_TRAY_UTIL_TRAY_LIST.at(i);

        if (atv == nullptr)
        {
            continue;
        }

        if (atv->id() == id)
        {
            result = HMC_TRAY_UTIL_TRAY_LIST.at(i);
        }
    }

    read_mutex.unlock();

    return result;
}

const HWND hmc_tray_util::hwnd()
{
    return m_Hwnd;
}

const int64_t hmc_tray_util::threadId()
{
    return int64_t(0) + m_ThreadId;
}

const std::thread *hmc_tray_util::thread()
{
    return m_TrayWorkerThread;
}

bool hmc_tray_util::setClassName(const std::wstring m_ClassName)
{
    if (this->m_Hwnd != NULL)
    {
        return false;
    }

    this->m_ClassName.append(m_ClassName.begin(), m_ClassName.end());
    return true;
}
