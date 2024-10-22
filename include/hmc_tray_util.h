#pragma once

// 防止重复导入
#ifndef MODE_INTERNAL_INCLUDE_HMC_TRAY_UTIL_HPP
#define MODE_INTERNAL_INCLUDE_HMC_TRAY_UTIL_HPP

#include <vector>
#include <string>
#include <thread>
#include <windows.h>
#include <functional>
#include <map>


class hmc_tray_util
{

public:
    NOTIFYICONDATAW nid{};

    using CALLBACK_ON_EVENT = std::unordered_map<std::wstring, std::vector<std::function<void()>>>;

    // 按钮类型
    enum class chMenuType
    {
        check = MF_CHECKED,
        separator = MF_SEPARATOR,
        // radio = MFT_RADIOCHECK,
        menu = 60409,
    };

    // 按钮
    struct chMenuItem
    {
        // 显示名称
        std::wstring name;
        // js的文本id
        std::wstring id;
        // 自动分配的id 留 0
        int messageID;
        // 按钮类型
        chMenuType type;
        // 禁用
        bool disable;
        // 子按钮列表
        std::vector<int> menuList;
        // 主按钮 如果是子按钮 不会被显示出来  只有主按钮中包含此键的id才会被包含到子按钮里显示
        bool rootMenu;
        // 是否显示此按钮
        bool show;
        // 是否选定
        bool select;
        chMenuItem() : type(chMenuType::menu), messageID(0), disable(false), rootMenu(false), show(false), select(false) { __initStore(); };
        void __initStore();
        /**
         * @brief 创建一个按钮
         *
         * @param name
         * @param id
         * @param disable
         * @return chMenuItem
         */
        static chMenuItem menu(std::wstring name, std::wstring id, bool disable = false);
        /**
         * @brief 创建一个带有选项的按钮
         *
         * @param name
         * @param id
         * @param disable
         * @return chMenuItem
         */
        static chMenuItem check(std::wstring name, std::wstring id, bool select = false);
        /**
         * @brief 创建一个分割线按钮
         *
         * @param id
         * @param root
         * @return chMenuItem
         */
        static chMenuItem separator(std::wstring id, bool root = true);
    };

    /**
     * @brief 监听
     *
     * @param eventName
     * @param fnc
     */
    void on(std::wstring eventName, std::function<void()> fnc);
    /**
     * @brief 单次监听
     *
     * @param eventName
     * @param fnc
     */
    void once(std::wstring eventName, std::function<void()> fnc);
    /**
     * @brief 设置托盘图标
     *
     * @param hNewIcon 图标
     * @param index 索引值
     * @return * BOOL
     */
    bool setTrayIcon(HICON hNewIcon, int index = 0);
    /**
     * @brief 修改tray的图标（从当前可执行文件获取）
     *
     * @return BOOL
     */
    bool setTrayIcon(int index = 0);
    /**
     * @brief 设置托盘图标
     *
     * @param hNewIcon 图标
     * @param index 索引值
     * @return * BOOL
     */
    bool setTrayIcon(std::wstring Icons, int index = 0);
    /**
     * @brief 启动
     *
     */
    bool start();
    // 结束托盘
    bool close();
    /**
     * @brief 添加按钮
     *
     * @param menuItem
     * @return true
     * @return false
     */
    bool addMenuItem(chMenuItem menuItem);

    /**
     * @brief 修改托盘显示的消息内容
     *
     * @param trayInfo
     * @return BOOL
     */
    bool setTrayInfo(std::wstring trayInfo);

    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param MenuId
     * @param RootMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(std::wstring RootMenuId, std::wstring SubMenuId);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(chMenuItem RootMenuId, std::wstring SubMenuId);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(chMenuItem RootMenuId, chMenuItem SubMenuId);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(std::wstring RootMenuId, chMenuItem SubMenuId);

    /**
     * @brief 设置指定按钮可见性
     *
     * @param MenuId
     * @param Visible
     * @return true
     * @return false
     */
    bool setMenuItmeVisible(std::wstring MenuId, bool Visible);
    // 设置按钮为禁用
    bool setMenuItmeEnable(std::wstring MenuId, bool Enable = true);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmeName(std::wstring MenuId, std::wstring Name);
    /**
     * @brief 设置指定的按钮为XXX的选择
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmeSelect(std::wstring MenuId, bool Select);
    /**
     * @brief 使用id获取按钮
     *
     * @param MenuId
     * @return chMenuItem
     */
    chMenuItem getMenuItme(std::wstring MenuId);

    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */

    bool setMenuItmetoSubMenu(const std::wstring RootMenuId, const std::vector<std::wstring> SubMenuIdArgs);

    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */

    bool setMenuItmetoSubMenu(chMenuItem RootMenuId, const std::vector<std::wstring> SubMenuIdArgs);
    /**
     * @brief 设置伪窗口的类名
     * 
     * @param m_ClassName 
     * @return true 
     * @return false 
     */
    bool setClassName(const std::wstring m_ClassName);
    const size_t id();
    /**
     * @brief 按照查询指定的托盘
     * 
     * @param id 
     * @return hmc_tray_util* 
     */
    static hmc_tray_util * at(size_t id);
    const HWND hwnd();
    const int64_t threadId();
    const std::thread * thread();
    static void GC_ALL();
private:
    HWND m_Hwnd = nullptr;
    int64_t m_hwndID = 0;
    int64_t m_ThreadId = 0;
    std::wstring m_TrayInfo = L"HMC_TRAY_UTIL_CLASS_ICON";
    std::wstring m_Title = L"HMC_TRAY_UTIL_CLASS_ICON";
    std::wstring m_ClassName = L"HMC_TRAY_UTIL_CLASS_ICON";
    bool m_StartTray = false;
    long long m_TrayNextId = 0;
    size_t m_id = 0;
    std::vector<chMenuItem> m_MenuList;
    std::thread *m_TrayWorkerThread = nullptr;

    CALLBACK_ON_EVENT ON_EVENT;
    CALLBACK_ON_EVENT ONCE_EVENT;

    static LRESULT CALLBACK c_CallBackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    int64_t c_openPuppetTrayWindow();

    /**
     * @brief 处理事件
     *
     * @param lParam
     */
    void emit(LPARAM lParam);
    /**
     * @brief 处理按钮响应
     *
     * @param keyID
     */
    void emit(std::wstring keyID, hmc_tray_util::chMenuItem &menuItem);

    // 点击了按钮
    void clickButtonItem(WORD IDKey);
};

extern std::vector<hmc_tray_util *> HMC_TRAY_UTIL_TRAY_LIST;


#endif // MODE_INTERNAL_INCLUDE_HMC_TRAY_UTIL_HPP