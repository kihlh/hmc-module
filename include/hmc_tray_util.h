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
#include <unordered_set>

class hmc_tray_util
{

public:
    hmc_tray_util();
    NOTIFYICONDATAW nid{};
    static std::vector<wchar_t> G_IGNORES_CHAR_LIST /*{'\0'}*/;
    static std::vector<std::wstring> G_FROZEN_EVENT_NAME_LIST /*{'\0'}*/;
    struct Menu;

    enum class Event
    {
        /**按钮的选中的值变化了 */
        select = 723521,
        /**按钮的显示的值变化了 */
        visible = 723522,
        /**按钮的显示的值变化了 */
        enable = 723523,
        /**中键点击 */
        middleClick = WM_MBUTTONDOWN,
        /**鼠标移动 */
        move = WM_MOUSEMOVE,
        /**左键单击 */
        click = WM_LBUTTONUP,
        /**按钮被按下 */
        clickBtn = 723525,
        /**鼠标移动 */
        mouseMove = WM_MOUSEMOVE,
        /**左键双击 */
        leftButtonDoubleClick = WM_LBUTTONDBLCLK,
        /**双击 */
        dblclick = WM_LBUTTONDBLCLK,
        /**左键释放 */
        leftButtonUp = WM_LBUTTONUP,
        /**左键按下 */
        leftButtonDown = WM_LBUTTONDOWN,
        /**右键释放 */
        rightButtonUp = WM_RBUTTONUP,
        /**右键按下 */
        rightButtonDown = WM_RBUTTONDOWN,
        /**中键按下 */
        middleClickDown = WM_MBUTTONDOWN,
        /**中键释放 */
        middleClickUp = WM_MBUTTONUP,
    };


    using CALLBACK_ENENT = std::function<void(hmc_tray_util *tray, Event EventID, const std::wstring &envName, Menu *menu)>;
    using CALLBACK_ON_EVENT = std::unordered_map<std::wstring, std::vector<CALLBACK_ENENT>>;

    // 按钮类型
    enum class chMenuType
    {
        check = MF_CHECKED,
        separator = MF_SEPARATOR,
        radio = MFT_RADIOCHECK,
        menu = 60409,
    };

   
    // 按钮
    struct Menu
    {
        // 显示名称
        std::wstring name;
        // js的文本id
        std::wstring id;
        // 自动分配的id 留 0
        int mid;
        // 按钮类型
        chMenuType type;
        // 禁用
        bool disable;
        // 子按钮列表
        std::unordered_set<int> menuList;
        // 禁止冲突选择列表
        std::unordered_set<size_t> uniqueSelectList;
        // 主按钮 如果是子按钮 不会被显示出来  只有主按钮中包含此键的id才会被包含到子按钮里显示
        bool rootMenu;
        // 是否显示此按钮
        bool show;
        // 是否选定
        bool select;

        HICON theIcon;
        
        bool setIcon(int index);
        bool setIcon();
        bool setIcon(HICON hNewIcon);
        bool setIconByIcon(std::wstring IconPath);
        bool setIconByExtract(std::wstring Icons, int index);
        void removeIcon();
        Menu() : type(chMenuType::menu), mid(-1), disable(false), rootMenu(false), show(false), select(false) ,theIcon(nullptr) { chMenuUtil::__initStore(this); };
        // if
        operator bool() const;
        // !
        bool operator!() const;
    };

    /**
     * @brief 监听
     *
     * @param eventName 事件名称
     * @param - ${idName}                  按钮的点击事件 如 `quit`
     * @param - ${idName}@click            按钮的点击事件 如 `quit@click`
     * @param - ${idName}@select           按钮的选中的值变化了
     * @param - ${idName}@visible          按钮的显示的值变化了
     * @param - ${idName}@enable           按钮的显示的值变化了
     * 
     * @param :: 触发全部按钮的 事件响应 ::
     * @param - @click                     按钮的点击事件 如 `quit@click`
     * @param - @select                    按钮的选中的值变化了
     * @param - @visible                   按钮的显示的值变化了
     * @param - @enable                    按钮的显示的值变化了
     * 
     * @param - *                          所有事件都会发送一次回调 事件名为实际事件 如 `quit@click` `click`
     * @param :: 该方法还支持以下鼠标事件 ::
     * @param - middleClick                中键点击
     * @param - move                       鼠标移动
     * @param - click                      左键单击
     * @param - mouseMove                  鼠标移动
     * @param - leftButtonDoubleClick      左键双击
     * @param - dblclick                   双击
     * @param - leftButtonUp               左键释放
     * @param - leftButtonDown             左键按下
     * @param - rightButtonUp              右键释放
     * @param - rightButtonDown            右键按下
     * @param fnc 回调函数
     */
    void on(std::wstring eventName, CALLBACK_ENENT fnc);
    /**
     * @brief 监听 (只执行一次)
     *
     * @param eventName 事件名称
     * @param - ${idName}                  按钮的点击事件 如 `quit`
     * @param - ${idName}@click            按钮的点击事件 如 `quit@click`
     * @param - ${idName}@select           按钮的选中的值变化了
     * @param - ${idName}@visible          按钮的显示的值变化了
     * @param - ${idName}@enable           按钮的显示的值变化了
     * 
     * @param :: 触发全部按钮的 事件响应 ::
     * @param - @click                     任意按钮的点击事件 如 `quit@click`
     * @param - @select                    任意按钮的选中的值变化了
     * @param - @visible                   任意按钮的显示的值变化了
     * @param - @enable                    任意按钮的显示的值变化了
     * 
     * @param - *                          所有事件都会发送一次回调 事件名为实际事件 如 `quit@click` `click`
     * @param :: 该方法还支持以下鼠标事件 ::
     * @param - middleClick             中键点击
     * @param - move                    鼠标移动
     * @param - click                   左键单击
     * @param - mouseMove               鼠标移动
     * @param - leftButtonDoubleClick   左键双击
     * @param - dblclick                双击
     * @param - leftButtonUp            左键释放
     * @param - leftButtonDown          左键按下
     * @param - rightButtonUp           右键释放
     * @param - rightButtonDown         右键按下
     * @param fnc 回调函数
     */
    void once(std::wstring eventName, CALLBACK_ENENT fnc);

    /**
     * @brief 设置 如 `Radio` `Check` 的选择状态为唯一值 (其他反选)
     *
     * @param key
     * @param value
     */
    bool setUniqueSelect(const std::wstring &Key, const std::wstring &Key2);
    /**
     * @brief 关联主键与 如 `Radio` `Check` 的选择状态为唯一值 (其他反选)
     *
     * @param key
     * @param value
     */
    bool setUniqueSelect(const std::wstring &Key, const std::vector<std::wstring> &Key2, const bool &LinkAll = false);

    /**
     * @brief 关联所有 如 `Radio` `Check` 的选择状态为唯一值 (其他反选)
     *
     * @param key
     * @param value
     */
    bool setUniqueSelect(const std::vector<std::wstring> &KeyList);

    /**
     * @brief 设置 如 `Radio` `Check` 的选择状态为唯一值 (其他反选)
     *
     * @param key
     * @param value
     */
    std::vector<Menu *> getUniqueSelect(const std::wstring &Key);
    /**
     * @brief 设置 如 `Radio` `Check` 的选择状态为唯一值 (其他反选)
     *
     * @param key
     * @param value
     */
    bool removeUniqueSelect(const std::wstring &Key, const std::wstring &Key2);

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
     * @param exePath 带图标的可执行文件
     * @param index 索引值
     * @return * BOOL
     */
    bool setTrayIconByExtract(std::wstring exePath, int index = 0);
    /**
     * @brief 设置托盘图标
     *
     * @param IconPath 图标图片
     * @param index 索引值
     * @return * BOOL
     */
    bool setTrayIconByIco(std::wstring IconPath);
    /**
     * @brief 启动
     *
     */
    bool start();
    /**
     * @brief 结束托盘并且释放内存
     *
     * @return true
     * @return false
     */
    bool close();
    /**
     * @brief 添加按钮
     *
     * @param menuItem
     * @return true
     * @return false
     */
    bool addMenuItem(Menu menuItem);

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
    bool setMenuItmetoSubMenu(Menu RootMenuId, std::wstring SubMenuId);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(Menu RootMenuId, Menu SubMenuId);
    /**
     * @brief 设置指定的按钮为XXX的子按钮
     *
     * @param RootMenuId
     * @param SubMenuId
     * @return true
     * @return false
     */
    bool setMenuItmetoSubMenu(std::wstring RootMenuId, Menu SubMenuId);

    /**
     * @brief 设置指定按钮可见性
     *
     * @param MenuId
     * @param Visible
     * @return true
     * @return false
     */
    bool setMenuItmeVisible(std::wstring MenuId, bool Visible);
    /**
     * @brief 设置按钮为禁用
     *
     * @param MenuId
     * @param Enable
     * @return true
     * @return false
     */
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
     * @return Menu
     */
    Menu &getMenuItme(std::wstring MenuId);
    Menu *getMenuItmePtr(std::wstring MenuId);

    /**
     * @brief 使用id获取按钮
     *
     * @param MenuId
     * @return Menu
     */
    Menu &getMenuItme(size_t MenuId);
    Menu *getMenuItmePtr(size_t MenuId);

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

    bool setMenuItmetoSubMenu(Menu RootMenuId, const std::vector<std::wstring> SubMenuIdArgs);
    /**
     * @brief 设置伪窗口的类名 start之前
     *
     * @param m_ClassName
     * @return true
     * @return false
     */
    bool setClassName(const std::wstring m_ClassName);
    /**
     * @brief 此托盘的唯一id
     * @example 可以用at找到托盘 如果不存在返回null
     *
     * @return const size_t
     */
    const size_t id();
    /**
     * @brief 按照查询指定的托盘 输入唯一id 查找托盘 如果不存在返回null
     *
     * @param id
     * @return hmc_tray_util*
     */
    static hmc_tray_util *at(size_t id);
    /**
     * @brief 傀儡窗口的句柄
     *
     * @return const HWND
     */
    const HWND hwnd();
    /**
     * @brief 执行傀儡任务的线程id
     *
     * @return const int64_t
     */
    const int64_t threadId();
    /**
     * @brief 线程句柄
     *
     * @return const std::thread*
     */
    const std::thread *thread();

    /**
     * @brief 释放所有的托盘图标 并且清空内存
     *
     */
    static void GC_ALL();
    /**
     * @brief 判断指定子按钮是否存在
     * 
     * @param MainId 父按钮
     * @param id 子按钮
     * @return true 
     * @return false 
     */
    const bool hasMenu(const std::wstring MainId, const std::wstring id);
    /**
     * @brief 判断指定按钮是否存在
     * 
     * @param id 
     * @return true 
     * @return false 
     */
    const bool hasMenu(const std::wstring id);
    /**
     * @brief 添加一个新的按钮 普通的按钮
     * 
     * @param name 显示文本
     * @param id id 必须是唯一的 用于查询和控制
     * @param disable 是否禁用
     * @return Menu* 
     */
    Menu *insertMenu(const std::wstring name, std::wstring id, const bool disable = false);
    /**
     * @brief 添加一个选项按钮 显示 √ 
     * 
     * @param name 名称
     * @param id id 必须是唯一的 用于查询和控制
     * @param select 显示时是否选中(如果调用了 setUniqueSelect 可能会自动替换)
     * @return Menu* 
     */
    Menu *insertCheck(const std::wstring name, std::wstring id, const bool select = false);
    /**
     * @brief 添加一个选项按钮 显示 ● 的
     * 
     * @param name 名称
     * @param id id 必须是唯一的 用于查询和控制
     * @param select 显示时是否选中(如果调用了 setUniqueSelect 可能会自动替换)
     * @return Menu* 
     */
    Menu *insertRadio(const std::wstring name, std::wstring id, const bool select = false);
    /**
     * @brief 一条分割线会自动加在之前元素后面 可以用来分割菜单 子菜单请使用insertSubSeparator
     * 
     * @param id 需要操控请自定义 默认分配顺序id
     * @return Menu* 
     */
    Menu *insertSeparator(const std::wstring id = L"");
    /**
     * @brief [子按钮]添加一个新的按钮 普通的按钮
     * 
     * @param MainId 父控件的id
     * @param name 显示文本
     * @param id id 必须是唯一的 用于查询和控制
     * @param disable 是否禁用
     * @return Menu* 
     */
    Menu *insertSubMenu(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool disable = false);
    /**
     * @brief [子按钮]添加一个选项按钮 显示 √ 
     * 
     * @param MainId 父控件的id
     * @param name 显示文本
     * @param id id 必须是唯一的 用于查询和控制
     * @param select 初始化时是否选中(如果调用了 setUniqueSelect 可能会自动替换)
     * @return Menu* 
     */
    Menu *insertSubCheck(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool select = false);
    /**
     * @brief [子按钮]添加一个选项按钮 显示 ● 的
     * 
     * @param MainId 父控件的id
     * @param name 显示文本
     * @param id id 必须是唯一的 用于查询和控制
     * @param select 初始化时是否选中(如果调用了 setUniqueSelect 可能会自动替换)
     * @return Menu* 
     */
    Menu *insertSubRadio(const std::wstring MainId, const std::wstring name, const std::wstring id, const bool select = false);
    /**
     * @brief  一条分割线会自动加在之前元素后面 可以用来分割菜单
     * 
     * @param MainId 父控件的id
     * @param id 需要操控请自定义 默认分配顺序id
     * @return Menu* 
     */
    Menu *insertSubSeparator(const std::wstring MainId, const std::wstring id = L"");

    std::vector<Menu *> getMenuList();

private:
    // 当前窗口的句柄
    HWND m_Hwnd = nullptr;
    // 当前窗口句柄id
    int64_t m_hwndID = 0;
    // 当前线程的id
    int64_t m_ThreadId = 0;
    // 当前托盘的图标显示的文本
    std::wstring m_TrayInfo = L"";
    // 傀儡窗口的标题
    std::wstring m_Title = L"";
    // 托盘傀儡窗口的类名 不能为空
    std::wstring m_ClassName = L"HMC_TRAY_UTIL_CLASS_ICON";
    // 当前图标的路径
    std::wstring m_TheIcon = L"";
    // 当前图标的类型 5 ico 1 exe 2 程序自带(当前可执行文件) 3 程序自带的共享缓冲区
    int m_TheIconType = 0;
    // 托盘的图标是否已经打开
    bool m_TheIconOpen = false;
    // 当前图标的索引
    size_t m_TheIconIndex = 0;
    // 托盘图标是否已经启动
    bool m_StartTray = false;
    // 当前按钮的id 用于生成唯一id
    size_t m_TrayMenuNextId = 1;
    // 托盘的id 用于生成唯一id
    size_t m_id = 0;
    // 所有按钮列表
    std::vector<Menu> m_MenuList;
    // 线程句柄
    std::thread *m_TrayWorkerThread = nullptr;
    // 事件回调列表
    CALLBACK_ON_EVENT ON_EVENT;
    // 单次事件回调列表
    CALLBACK_ON_EVENT ONCE_EVENT;

    static LRESULT CALLBACK c_CallBackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    int64_t c_openPuppetTrayWindow();

    /**
     * @brief 显示菜单
     *
     */
    static void ShowTrackPopupMenu(hmc_tray_util *trayPtr, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void $HMC_EMIT_ENVINFO(const std::wstring assert_IF, Event EventID);

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
    void emit(std::wstring keyID,Event EventID, hmc_tray_util::Menu &menuItem);
    
    void gcIcon();

    // 点击了按钮
    void clickButtonItem(WORD IDKey);
    
    void initNotifyIconData();

    class chMenuUtil
    {
    public:
        /**
         * @brief 创建一个按钮
         *
         * @param name 显示名称
         * @param id 控件唯一id 请确保唯一性不然会导致按钮寻找到其他的
         * @param disable 是否禁用
         * @return Menu
         */
        static Menu menu(std::wstring name, std::wstring id, bool disable = false);
        /**
         * @brief 创建一个带有选项的按钮
         *
         * @param name 显示名称
         * @param id 控件唯一id 请确保唯一性不然会导致按钮寻找到其他的
         * @param disable 是否禁用
         * @return Menu
         */
        static Menu check(std::wstring name, std::wstring id, bool select = false);
        /**
         * @brief 创建一个带有圆圈选项的按钮
         *
         * @param name 显示名称
         * @param id 控件唯一id 请确保唯一性不然会导致按钮寻找到其他的
         * @param disable 是否禁用
         * @return Menu
         */
        static Menu radio(std::wstring name, std::wstring id, bool select = false);
        /**
         * @brief 创建一个分割线按钮
         *
         * @param id 控件唯一id 请确保唯一性不然会导致按钮寻找到其他的
         * @param root 是否立即显示在根目录 (为false可以创建但是不显示)
         * @return Menu
         */
        static Menu separator(std::wstring id, bool root = true);

        static void __initStore(Menu *input);
    };

    static Menu MenuNot;
};

extern std::vector<hmc_tray_util *> HMC_TRAY_UTIL_TRAY_LIST;

#endif // MODE_INTERNAL_INCLUDE_HMC_TRAY_UTIL_HPP