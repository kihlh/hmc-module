#include "hmc_screen_util.h"
#include "hmc_util.h"
#include "hmc_tray_util.h"
#include "hmc_string_util.h"
#include <unordered_map>
#include "hmc_process_util.h"
#include <shared_mutex>
#include <atomic>

std::vector<hmc_tray_util *> HMC_TRAY_UTIL_TRAY_LIST;
std::shared_mutex GpSharedMutex;
std::atomic<size_t> GpTaryID = 1;

hmc_tray_util::Menu hmc_tray_util::MenuNot = hmc_tray_util::Menu();

std::vector<wchar_t> hmc_tray_util::G_IGNORES_CHAR_LIST = {'\0'};
std::vector<std::wstring> hmc_tray_util::G_FROZEN_EVENT_NAME_LIST = {};

#define WM_SYSICON (WM_USER + 1)

// if
hmc_tray_util::Menu::operator bool() const
{
    return mid > 0;
}

// !
bool hmc_tray_util::Menu::operator!() const
{
    return mid <= 0;
}

void hmc_tray_util::chMenuUtil::__initStore(Menu *input)
{
    auto &item = *input;

    item.mid = -1;
    item.select = false;
    item.menuList = {};
    item.disable = false;
    item.rootMenu = true;
    item.show = true;
    item.name = L"unknown";
    item.id = L"unknown" + std::to_wstring(item.mid);
    item.type = chMenuType::menu;
};

hmc_tray_util::Menu hmc_tray_util::chMenuUtil::menu(std::wstring name, std::wstring id, bool disable)
{
    hmc_tray_util::Menu menuItem;
    menuItem.disable = disable;
    menuItem.id.clear();
    menuItem.id.append(id.begin(), id.end());
    hmc_string_util::removeMatchChar(&menuItem.id, hmc_tray_util::G_IGNORES_CHAR_LIST);

    menuItem.name = name;
    menuItem.rootMenu = true;
    menuItem.menuList = {};
    menuItem.show = true;
    menuItem.select = false;
    menuItem.type = hmc_tray_util::chMenuType::menu;
    return menuItem;
}

hmc_tray_util::Menu hmc_tray_util::chMenuUtil::check(std::wstring name, std::wstring id, bool select)
{
    hmc_tray_util::Menu menuItem;
    menuItem.disable = false;
    menuItem.id = std::wstring(id.begin(), id.end());
    hmc_string_util::removeMatchChar(&menuItem.id, hmc_tray_util::G_IGNORES_CHAR_LIST);

    menuItem.name = name;
    menuItem.rootMenu = true;
    menuItem.menuList = {};
    menuItem.show = true;
    menuItem.select = select;
    menuItem.type = hmc_tray_util::chMenuType::check;
    return menuItem;
}

hmc_tray_util::Menu hmc_tray_util::chMenuUtil::radio(std::wstring name, std::wstring id, bool select)
{
    hmc_tray_util::Menu menuItem;
    menuItem.disable = false;

    menuItem.id = std::wstring(id.begin(), id.end());
    hmc_string_util::removeMatchChar(&menuItem.id, hmc_tray_util::G_IGNORES_CHAR_LIST);

    menuItem.name = name;
    menuItem.rootMenu = true;
    menuItem.menuList = {};
    menuItem.show = true;
    menuItem.select = select;
    menuItem.type = hmc_tray_util::chMenuType::radio;
    return menuItem;
}

hmc_tray_util::Menu hmc_tray_util::chMenuUtil::separator(std::wstring id, bool root)
{
    hmc_tray_util::Menu menuItem;
    menuItem.disable = false;

    if (!id.empty())
    {
        menuItem.id = std::wstring(id.begin(), id.end());
    }

    hmc_string_util::removeMatchChar(&menuItem.id, hmc_tray_util::G_IGNORES_CHAR_LIST);

    menuItem.show = true;
    menuItem.rootMenu = root;
    menuItem.menuList = {};
    menuItem.select = false;
    menuItem.type = hmc_tray_util::chMenuType::separator;
    return menuItem;
}

void hmc_tray_util::$HMC_EMIT_ENVINFO(const std::wstring assert_IF, Event EventID)
{
    const std::wstring AS_ALL_KEY(1, '*');

    if (ON_EVENT.find(assert_IF) != ON_EVENT.end())
    {
        auto &fun_list = ON_EVENT[assert_IF];

        for (auto &fun : fun_list)
        {
            fun(this, EventID, assert_IF, nullptr);
        }
    }

    if (ON_EVENT.find(AS_ALL_KEY) != ON_EVENT.end())
    {
        auto &fun_list = ON_EVENT[AS_ALL_KEY];

        for (auto &fun : fun_list)
        {
            fun(this, EventID, assert_IF, nullptr);
        }
    }

    if (ONCE_EVENT.find(assert_IF) != ONCE_EVENT.end())
    {
        auto &fun_list = ONCE_EVENT[assert_IF];

        for (auto &fun : fun_list)
        {
            fun(this, EventID, assert_IF, nullptr);
        }

        ONCE_EVENT[assert_IF].clear();
    }

    if (ONCE_EVENT.find(AS_ALL_KEY) != ONCE_EVENT.end())
    {
        auto &fun_list = ONCE_EVENT[AS_ALL_KEY];

        for (auto &fun : fun_list)
        {
            fun(this, EventID, assert_IF, nullptr);
        }

        ONCE_EVENT[AS_ALL_KEY].clear();
    }
}

void hmc_tray_util::emit(LPARAM lParam)
{
    switch (lParam)
    {
        // 鼠标右键按下
    case WM_RBUTTONDOWN:
    {

        $HMC_EMIT_ENVINFO(L"rightButtonDown", Event(lParam));

        break;
    }
        // 鼠标右键松开
    case WM_RBUTTONUP:
    {
        $HMC_EMIT_ENVINFO(L"rightButtonUp", Event(lParam));
        break;
    }
        // 鼠标左键按下
    case WM_LBUTTONDOWN:
    {
        $HMC_EMIT_ENVINFO(L"leftButtonDown", Event(lParam));

        break;
    }
        // 鼠标左键松开
    case WM_LBUTTONUP:
    {
        $HMC_EMIT_ENVINFO(L"click", Event(lParam));
        $HMC_EMIT_ENVINFO(L"leftButtonUp", Event(lParam));

        break;
    }
        // 鼠标左键双击
    case WM_LBUTTONDBLCLK:
    {
        $HMC_EMIT_ENVINFO(L"dblclick", Event(lParam));
        $HMC_EMIT_ENVINFO(L"leftButtonDoubleClick", Event(lParam));

        break;
    }
        // 鼠标浮动
    case WM_MOUSEMOVE:
    {
        $HMC_EMIT_ENVINFO(L"move", Event(lParam));
        $HMC_EMIT_ENVINFO(L"mouseMove", Event(lParam));
        break;
    }
    // 中键点击
    case WM_MBUTTONDOWN:
    {
        $HMC_EMIT_ENVINFO(L"middleClick", Event(lParam));
        $HMC_EMIT_ENVINFO(L"middleClickDown", Event(lParam));
        break;
    }

    case WM_MBUTTONUP:
    {
        $HMC_EMIT_ENVINFO(L"middleClickUp", Event(lParam));
        break;
    }
    }
}

void hmc_tray_util::emit(std::wstring keyID, Event EventID, hmc_tray_util::Menu &menuItem)
{
    std::vector<std::wstring> ALL_KEY_LIST = {keyID, std::wstring(1, '*')};

    if (menuItem)
    {

        switch (EventID)
        {
        case Event::select:
            ALL_KEY_LIST.push_back(std::wstring(L"@select"));
            break;

        case Event::clickBtn:
            ALL_KEY_LIST.push_back(std::wstring(L"@click"));
            break;

        case Event::visible:
            ALL_KEY_LIST.push_back(std::wstring(L"@visible"));
            break;

        case Event::enable:
            ALL_KEY_LIST.push_back(std::wstring(L"@enable"));
            break;

        default:
            break;
        }
    }

    for (size_t i = 0; i < ALL_KEY_LIST.size(); i++)
    {
        auto &AS_ALL_KEY = ALL_KEY_LIST[i];

        if (ON_EVENT.contains(AS_ALL_KEY))
        {
            auto &eventList = ON_EVENT.at(AS_ALL_KEY);

            for (size_t i = 0; i < eventList.size(); i++)
            {
                if (auto &fun = eventList[i])
                {
                    fun(this, EventID, keyID, &menuItem);
                };
            }
        }
    }

    for (size_t i = 0; i < ALL_KEY_LIST.size(); i++)
    {
        auto &AS_ALL_KEY = ALL_KEY_LIST[i];
        if (ONCE_EVENT.contains(AS_ALL_KEY))
        {

            auto &eventList = ONCE_EVENT.at(AS_ALL_KEY);

            for (size_t i = 0; i < eventList.size(); i++)
            {
                if (auto &fun = eventList[i])
                {
                    fun(this, EventID, keyID, &menuItem);
                };
            }

            ONCE_EVENT.at(AS_ALL_KEY).clear();
        }
    }
}

void hmc_tray_util::clickButtonItem(WORD IDKey)
{
    std::wstring id = std::wstring();
    const auto check = hmc_tray_util::chMenuType::check;
    const auto radio = hmc_tray_util::chMenuType::radio;

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.mid == IDKey)
        {
            id.clear();
            id.append(Menu.id);

            emit(std::wstring(id).append(L"@click"), Event::clickBtn, m_MenuList[i]);

            if (Menu.type == check || Menu.type == radio)
            {
                bool select = !Menu.select;
                m_MenuList[i].select = select;

                emit(std::wstring(id).append(L"@select"), Event::select, m_MenuList[i]);

                // 重置其他冲突选项 的反选
                if (Menu.uniqueSelectList.size())
                {
                    for (auto &&item : Menu.uniqueSelectList)
                    {
                        auto ptr = getMenuItmePtr(item);

                        if (ptr != nullptr && ptr != &Menu)
                        {
                            if (ptr->select != !select)
                            {
                                ptr->select = !select;
                                emit(std::wstring(ptr->id).append(L"@select"), Event::select, *ptr);
                            }
                        }
                    }
                }

                m_MenuList[i].select = select;
            }

            emit(id, Event::click, m_MenuList[i]);
        }
    }
}

/**
 * @brief 监听
 *
 * @param eventName
 * @param fnc
 */
void hmc_tray_util::on(std::wstring eventName, CALLBACK_ENENT fnc)
{

    std::vector<CALLBACK_ENENT> funList = {};

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
void hmc_tray_util::once(std::wstring eventName, CALLBACK_ENENT fnc)
{
    std::vector<CALLBACK_ENENT> funList = {};

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

        gcIcon();
        m_TheIconOpen = false;

        m_TheIconIndex = index;
        m_TheIcon.clear();
        m_TheIconType = 3;

        nid.hIcon = hNewIcon;
        return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
    HMC_CATCH_ALL;

    return false;
}

bool hmc_tray_util::setTrayIconByIco(std::wstring IconPath)
{
    try
    {

        HICON hWindowIcon = (HICON)::LoadImageW(GetModuleHandle(NULL), IconPath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        if (hWindowIcon != NULL)
        {

            gcIcon();
            nid.hIcon = hWindowIcon;
            m_TheIconOpen = true;

            m_TheIconIndex = 0;
            m_TheIcon = std::wstring(IconPath);
            m_TheIconType = 5;

            return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
    }
    HMC_CATCH_ALL;

    return false;
}

bool hmc_tray_util::setTrayIconByExtract(std::wstring Icons, int index)
{
    try
    {
        HICON hNewIcon = NULL; // 声明一个HICON句柄
        hNewIcon = ::ExtractIconW(NULL, Icons.c_str(), (UINT)index);
        if (hNewIcon != NULL)
        {

            gcIcon();
            nid.hIcon = hNewIcon;
            m_TheIconOpen = true;

            m_TheIconIndex = index;
            m_TheIcon = std::wstring(Icons);
            m_TheIconType = 1;
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

        gcIcon();

        nid.hIcon = hNewIcon;
        m_TheIconOpen = true;
        m_TheIconIndex = index;
        m_TheIcon = std::move(the_path);
        m_TheIconType = 2;
        return ::Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
    HMC_CATCH_ALL;
    return false;
}

void hmc_tray_util::GC_ALL()
{
    if (HMC_TRAY_UTIL_TRAY_LIST.empty())
    {
        return;
    }

    for (auto &&at : HMC_TRAY_UTIL_TRAY_LIST)
    {
        if (at != nullptr)
            at->close();
    }
}

hmc_tray_util::hmc_tray_util()
{
    if (hmc_tray_util::G_FROZEN_EVENT_NAME_LIST.empty())
    {

        const std::vector<std::wstring> G_LIST = {
            L"rightButtonDown", L"rightButtonUp", L"leftButtonDown",
            L"click", L"leftButtonUp", L"dblclick", L"leftButtonDoubleClick",
            L"move", L"mouseMove", L"middleClick", L"middleClickDown",
            L"middleClickUp", L"@click", L"@select", L"@visible", L"@enable"};

        for (auto &&at : G_LIST)
        {
            hmc_tray_util::G_FROZEN_EVENT_NAME_LIST.push_back(at);
        }
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
bool hmc_tray_util::addMenuItem(hmc_tray_util::Menu menuItem)
{
    if (hasMenu(menuItem.id))
    {
        return false;
    }

    int idkey = static_cast<int>(m_TrayMenuNextId++);

    menuItem.mid = idkey;

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

void hmc_tray_util::ShowTrackPopupMenu(hmc_tray_util *trayPtr, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
                        ::InsertMenuW(hMenu, -1, MF_BYPOSITION, Menu.mid, Menu.name.c_str());
                    }
                    // 分割线
                    else if (Menu.type == hmc_tray_util::chMenuType::separator)
                    {
                        ::InsertMenuW(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, Menu.mid, Menu.name.c_str());
                    }
                    // 选项
                    else if (Menu.type == hmc_tray_util::chMenuType::check)
                    {
                        if (Menu.select)
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MF_CHECKED, Menu.mid, Menu.name.c_str());
                        }
                        else
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MF_UNCHECKED, Menu.mid, Menu.name.c_str());
                        }
                    }
                    // 选项
                    else if (Menu.type == hmc_tray_util::chMenuType::radio)
                    {
                        if (Menu.select)
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MFT_RADIOCHECK | MF_CHECKED, Menu.mid, Menu.name.c_str());
                        }
                        else
                        {
                            ::InsertMenuW(hMenu, -1, MF_BYCOMMAND | MFT_RADIOCHECK | MF_UNCHECKED, Menu.mid, Menu.name.c_str());
                        }
                    }

                    // 禁用
                    if (Menu.disable)
                    {
                        ::EnableMenuItem(hMenu, Menu.mid, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                    }
                }
                // 有子目录的按钮
                else if (Menu.show && Menu.rootMenu && Menu.menuList.size() != 0)
                {
                    ::InsertMenuW(hMenu, -1, MF_BYPOSITION, Menu.mid, Menu.name.c_str());
                    HMENU hSubMenu = ::CreatePopupMenu();
                    std::shared_ptr<void> close_hSubMenu(nullptr, [&](void *)
                                                         {if (hSubMenu != nullptr) {try{::DestroyMenu(hSubMenu);}HMC_CATCH_ALL;} });
                    if (!Menu.show)
                        continue;

                    for (auto &&IDmid : Menu.menuList)
                    {
                        // 创建子菜单
                        if (hSubMenu)
                        {
                            for (auto &&_Menu : trayPtr->m_MenuList)
                            {
                                if (_Menu.mid != IDmid)
                                    continue;

                                // 按钮
                                if (_Menu.type == hmc_tray_util::chMenuType::menu)
                                {
                                    ::AppendMenuW(hSubMenu, MF_STRING, _Menu.mid, _Menu.name.c_str());

                                    // ::InsertMenuW(hMenu, -1, MF_BYPOSITION, _Menu.mid, _Menu.name.c_str());
                                }
                                // 分割线
                                else if (_Menu.type == hmc_tray_util::chMenuType::separator)
                                {
                                    ::AppendMenuW(hSubMenu, MF_SEPARATOR, _Menu.mid, _Menu.name.c_str());
                                    // ::InsertMenuW(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, _Menu.mid, _Menu.name.c_str());
                                }
                                // 选项
                                else if (_Menu.type == hmc_tray_util::chMenuType::check)
                                {
                                    if (_Menu.select)
                                    {
                                        ::AppendMenuW(hSubMenu, MF_CHECKED, _Menu.mid, _Menu.name.c_str());
                                    }
                                    else
                                    {
                                        ::AppendMenuW(hSubMenu, MF_UNCHECKED, _Menu.mid, _Menu.name.c_str());
                                    }
                                }
                                else if (_Menu.type == hmc_tray_util::chMenuType::radio)
                                {
                                    if (_Menu.select)
                                    {
                                        ::AppendMenuW(hSubMenu, MFT_RADIOCHECK | MF_CHECKED, _Menu.mid, _Menu.name.c_str());
                                    }
                                    else
                                    {
                                        ::AppendMenuW(hSubMenu, MFT_RADIOCHECK | MF_UNCHECKED, _Menu.mid, _Menu.name.c_str());
                                    }
                                }

                                // 禁用
                                if (_Menu.disable)
                                {
                                    ::EnableMenuItem(hSubMenu, _Menu.mid, MF_DISABLED | MF_GRAYED);
                                }
                            }

                            // 把子菜单加到主菜单项
                            ::ModifyMenuW(hMenu, Menu.mid, MF_POPUP, (UINT_PTR)hSubMenu, Menu.name.c_str());
                        }
                    }

                    // 这让菜单显示为需要的样子，例如处理具有复选菜单项的菜单
                    ::SetMenuDefaultItem(hMenu, Menu.mid, FALSE);
                }
            }
            ::TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, curPoint.x, curPoint.y, 0, hWnd, NULL);
            ::DestroyMenu(hMenu);
        }
    }
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
    {

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
                trayPtr->m_TheIconOpen = false;
                trayPtr->m_TheIconIndex = 0;
                trayPtr->m_TheIcon.clear();
                trayPtr->m_TheIconType = 3;
            }

            ::wcsncpy_s(trayPtr->nid.szTip, trayPtr->m_Title.c_str(), trayPtr->m_Title.size());

            trayPtr->nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

            ::Shell_NotifyIconW(NIM_ADD, &((*trayPtr).nid));
        }

        break;
    }
    case WM_DESTROY:
    {

        if (trayPtr != nullptr)
        {
            trayPtr->gcIcon();

            ::Shell_NotifyIconW(NIM_DELETE, &((*trayPtr).nid));
        }
        ::KillTimer(hWnd, 1);
        ::KillTimer(hWnd, 2);
        ::PostQuitMessage(0);
        break;
    }

    case WM_SYSICON:
    {

        if (trayPtr != nullptr)
        {
            trayPtr->emit(lParam);
            ShowTrackPopupMenu(trayPtr, hWnd, message, wParam, lParam);
        }

        break;
    }
    case WM_COMMAND:
    {
        if (trayPtr)
            trayPtr->clickButtonItem(LOWORD(wParam));
    }
    default:
    {
        if (hWnd != NULL)
        {
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }
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
    ::SetTimer((const HWND)m_Hwnd, 2, 1000 * 150, NULL);

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
        if (hmc_string_util::diff(menu.id, RootMenuId, G_IGNORES_CHAR_LIST) /**|| menu.id == SubMenuId */)
        {
            menu.rootMenu = true;
            for (auto &&menu2 : m_MenuList)
            {
                if (hmc_string_util::diff(menu2.id, SubMenuId, G_IGNORES_CHAR_LIST) /**|| menu2.id == SubMenuId */)
                {
                    menu2.rootMenu = false;
                    menu.menuList.insert(menu2.mid);
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
bool hmc_tray_util::setMenuItmetoSubMenu(Menu RootMenuId, std::wstring SubMenuId)
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
bool hmc_tray_util::setMenuItmetoSubMenu(Menu RootMenuId, Menu SubMenuId)
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
bool hmc_tray_util::setMenuItmetoSubMenu(std::wstring RootMenuId, Menu SubMenuId)
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

bool hmc_tray_util::setMenuItmetoSubMenu(Menu RootMenuId, const std::vector<std::wstring> SubMenuIdArgs)
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
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
        {
            auto the_show = Menu.show;
            if (the_show != Visible)
            {
                emit(Menu.id, Event::visible, m_MenuList[i]);
                m_MenuList[i].show = Visible ? true : false;
                return true;
            }
            return false;
        }
    }
    return false;
}

// 设置按钮为禁用
bool hmc_tray_util::setMenuItmeEnable(std::wstring MenuId, bool Enable)
{
    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
        {
            auto the_disable = Menu.disable;
            if (the_disable != Enable)
            {
                emit(Menu.id, Event::enable, m_MenuList[i]);
                m_MenuList[i].disable = Enable ? true : false;
                return true;
            }
            return false;
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
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
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
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
        {
            auto the_select = Menu.select;
            if (the_select != Select)
            {
                emit(Menu.id, Event::select, m_MenuList[i]);
                m_MenuList[i].select = Select;
                return true;
            }
            return false;
        }
    }
    return false;
}

/**
 * @brief Get the Menu Itme object
 *
 * @param MenuId
 * @return Menu
 */
hmc_tray_util::Menu &hmc_tray_util::getMenuItme(std::wstring MenuId)
{

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
        {
            return m_MenuList[i];
        }
    }

    return MenuNot;
}

hmc_tray_util::Menu *hmc_tray_util::getMenuItmePtr(std::wstring MenuId)
{

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (hmc_string_util::diff(Menu.id, MenuId, G_IGNORES_CHAR_LIST))
        {
            return &m_MenuList.at(i);
        }
    }

    return nullptr;
}

hmc_tray_util::Menu &hmc_tray_util::getMenuItme(size_t MenuId)
{

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.mid == MenuId)
        {
            return m_MenuList[i];
        }
    }

    return MenuNot;
}

hmc_tray_util::Menu *hmc_tray_util::getMenuItmePtr(size_t MenuId)
{

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        if (Menu.mid == MenuId)
        {
            return &m_MenuList[i];
        }
    }

    return nullptr;
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

void hmc_tray_util::gcIcon()
{
    if (m_TheIconType == 3)
    {
        return;
    }

    if (m_TheIconOpen && nid.hIcon != nullptr)
    {

        ::DestroyIcon(nid.hIcon);
        nid.hIcon = nullptr;

        return;
    }
}
const bool hmc_tray_util::hasMenu(const std::wstring MainId, const std::wstring id)
{
    bool result = hasMenu(MainId);

    if (!result)
    {
        return result;
    }

    auto MainItem = this->getMenuItmePtr(MainId);
    auto SubMenu = this->getMenuItmePtr(id);

    if (MainItem != nullptr && SubMenu != nullptr)
    {
        if (MainItem->menuList.empty())
        {
            return false;
        }

        auto &subid = SubMenu->mid;
        for (auto &&id : MainItem->menuList)
        {
            if (id == subid)
            {
                return true;
            }
        }
    }

    return result;
}

const bool hmc_tray_util::hasMenu(const std::wstring id)
{
    bool result = false;

    for (size_t i = 0; i < hmc_tray_util::G_FROZEN_EVENT_NAME_LIST.size(); i++)
    {
        auto &eventName = hmc_tray_util::G_FROZEN_EVENT_NAME_LIST.at(i);
        if (hmc_string_util::diff(eventName, id, G_IGNORES_CHAR_LIST))
        {
            return true;
        }
    }

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &menu = m_MenuList.at(i);
        if (hmc_string_util::diff(menu.id, id, G_IGNORES_CHAR_LIST))
        {
            return true;
        }
    }

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertMenu(const std::wstring name, const std::wstring id, const bool disable)
{
    Menu *result = nullptr;
    if (hasMenu(id))
    {
        return result;
    }

    if (addMenuItem(chMenuUtil::menu(name, id, disable)))
    {
        return &m_MenuList.back();
    }

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertCheck(const std::wstring name, const std::wstring id, const bool select)
{
    Menu *result = nullptr;
    if (hasMenu(id))
    {
        return result;
    }

    if (addMenuItem(chMenuUtil::check(name, id, select)))
    {
        return &m_MenuList.back();
    }

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertRadio(const std::wstring name, const std::wstring id, const bool select)
{
    Menu *result = nullptr;
    if (hasMenu(id))
    {
        return result;
    }

    if (addMenuItem(chMenuUtil::radio(name, id, select)))
    {
        return &m_MenuList.back();
    }

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertSeparator(const std::wstring id)
{
    Menu *result = nullptr;
    if (hasMenu(id))
    {
        return result;
    }

    if (addMenuItem(chMenuUtil::separator(id)))
    {
        return &m_MenuList.back();
    }

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertSubMenu(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool disable)
{
    Menu *result = insertMenu(name, id, disable);
    if (result != nullptr)
    {
        result->rootMenu = false;
    }
    setMenuItmetoSubMenu(MainId, id);
    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertSubCheck(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool select)
{
    Menu *result = insertCheck(name, id, select);
    if (result != nullptr)
    {
        result->rootMenu = false;
    }
    setMenuItmetoSubMenu(MainId, id);

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertSubRadio(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool select)
{
    Menu *result = insertRadio(name, id, select);
    if (result != nullptr)
    {
        result->rootMenu = false;
    }
    setMenuItmetoSubMenu(MainId, id);

    return result;
}

hmc_tray_util::Menu *hmc_tray_util::insertSubSeparator(const std::wstring MainId, const std::wstring id)
{
    Menu *result = insertSeparator(id);
    if (result != nullptr)
    {
        result->rootMenu = false;
    }
    setMenuItmetoSubMenu(MainId, id);

    return result;
}

bool hmc_tray_util::setUniqueSelect(const std::wstring &Key, const std::wstring &Key2)
{
    auto MainItem = this->getMenuItmePtr(Key);
    auto MainItem2 = this->getMenuItmePtr(Key2);

    if (MainItem == nullptr || MainItem2 == nullptr)
    {
        return false;
    }

    bool select2 = MainItem2->select;
    bool select1 = MainItem->select;

    if (select2)
    {
        MainItem->select = false;
    }

    if (select1)
    {
        MainItem2->select = false;
    }

    MainItem2->uniqueSelectList.insert(MainItem->mid);
    MainItem->uniqueSelectList.insert(MainItem2->mid);

    return true;
}

bool hmc_tray_util::removeUniqueSelect(const std::wstring &Key, const std::wstring &Key2)
{
    auto MainItem = this->getMenuItmePtr(Key);
    auto MainItem2 = this->getMenuItmePtr(Key2);

    if (MainItem == nullptr || MainItem2 == nullptr)
    {
        return false;
    }

    MainItem2->uniqueSelectList.erase(MainItem->mid);
    MainItem->uniqueSelectList.erase(MainItem2->mid);

    return true;
}

std::vector<hmc_tray_util::Menu *> hmc_tray_util::getUniqueSelect(const std::wstring &Key)
{
    std::vector<hmc_tray_util::Menu *> result;

    auto MainItem = this->getMenuItmePtr(Key);
    if (MainItem == nullptr)
    {
        return result;
    }

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];

        if (MainItem->uniqueSelectList.find(Menu.mid) != MainItem->uniqueSelectList.end())
        {
            result.push_back(&Menu);
            continue;
        }
    }

    return result;
}

bool hmc_tray_util::setUniqueSelect(const std::vector<std::wstring> &KeyList)
{
    std::vector<Menu *> __MenuList;

    for (size_t i = 0; i < KeyList.size(); i++)
    {
        auto &Key = KeyList[i];
        auto MainItem2 = this->getMenuItmePtr(Key);

        if (MainItem2 == nullptr)
        {
            continue;
        }

        __MenuList.push_back(MainItem2);
    }

    if (__MenuList.empty())
    {
        return false;
    }

    // 关联所有
    for (size_t i = 0; i < __MenuList.size(); i++)
    {
        auto &at1 = __MenuList[i];
        if (at1 != nullptr)
        {

            for (size_t i3 = 0; i3 < __MenuList.size(); i3++)
            {
                auto &at = __MenuList[i3];
                if (at != nullptr)
                {
                    at1->uniqueSelectList.insert(at->mid);
                }
            }
        }
    }

    auto MainItemSelect = __MenuList[0];

    // 主key未启用 查找是否有启用的项

    for (size_t i = 0; i < __MenuList.size(); i++)
    {
        auto &at = __MenuList[i];

        if (at->select)
        {
            MainItemSelect = __MenuList[i];
            break;
        }
    }

    // 主key未启用 有启用的项（第一次）
    if (MainItemSelect != nullptr && MainItemSelect->select)
    {
        for (size_t i = 0; i < __MenuList.size(); i++)
        {
            auto &at = __MenuList[i];

            // 不处理自己
            if (MainItemSelect == at)
            {
                continue;
            }

            if (at->select)
            {
                at->select = false;
            }
        }

        MainItemSelect->select = true;

        return true;
    }

    if (!MainItemSelect->select)
    {
        MainItemSelect->select = false;
    }

    return true;
}

bool hmc_tray_util::setUniqueSelect(const std::wstring &Key, const std::vector<std::wstring> &Key2, const bool &LinkAll)
{
    auto MainItem = this->getMenuItmePtr(Key);

    if (MainItem == nullptr)
    {
        return false;
    }

    std::vector<Menu *> __MenuList;

    bool select1 = MainItem->select;

    for (size_t i = 0; i < Key2.size(); i++)
    {
        auto &Key2Item = Key2[i];
        auto MainItem2 = this->getMenuItmePtr(Key2Item);

        if (MainItem2 == nullptr)
        {
            continue;
        }

        __MenuList.push_back(MainItem2);

        MainItem->uniqueSelectList.insert(MainItem2->mid);
        MainItem2->uniqueSelectList.insert(MainItem->mid);
    }

    if (LinkAll)
        for (size_t i = 0; i < __MenuList.size(); i++)
        {
            auto &at1 = __MenuList[i];
            if (at1 != nullptr)
            {

                for (size_t i3 = 0; i3 < __MenuList.size(); i3++)
                {
                    auto &at = __MenuList[i3];
                    if (at != nullptr)
                    {
                        at1->uniqueSelectList.insert(at->mid);
                    }
                }
            }
        }

    // 开始查找带有启用的项 或者 禁用的项

    auto MainItemSelect = this->getMenuItmePtr(Key);

    // 主key已启用
    if (MainItemSelect != nullptr && MainItemSelect->select)
    {
        for (size_t i = 0; i < __MenuList.size(); i++)
        {
            auto &at = __MenuList[i];

            if (at->select)
            {
                at->select = false;
            }
        }

        MainItemSelect->select = true;

        return true;
    }

    // 主key未启用 查找是否有启用的项

    for (size_t i = 0; i < __MenuList.size(); i++)
    {
        auto &at = __MenuList[i];

        if (at->select)
        {
            MainItemSelect = __MenuList[i];
            break;
        }
    }

    // 主key未启用 有启用的项（第一次）
    if (MainItemSelect != nullptr && MainItemSelect->select)
    {
        for (size_t i = 0; i < __MenuList.size(); i++)
        {
            auto &at = __MenuList[i];

            // 不处理自己
            if (MainItemSelect == at)
            {
                continue;
            }

            if (at->select)
            {
                at->select = false;
            }
        }

        MainItemSelect->select = true;

        return true;
    }

    // 没有未启用 主key启用

    MainItem->select = true;

    return true;
}

std::vector<hmc_tray_util::Menu *> hmc_tray_util::getMenuList()
{
    std::vector<Menu *> result;

    for (size_t i = 0; i < m_MenuList.size(); i++)
    {
        auto &Menu = m_MenuList[i];
        result.push_back(&Menu);
    }

    return result;
}
