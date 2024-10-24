#include <fstream>
#include <string>
#include <windows.h>
#include <hmc_env_util.h>
#include <hmc_string_util.h>
#include <hmc_registr_util.h>
#include <hmc_windows_util.h>
#include <hmc-module.h>

#include <iostream>
#include <hmc_tray_util.h>

#ifdef _DEBUG  
#ifdef _WIN64 
// 如果发现无法找到静态文件可以测试以下方法(其中一种)
// 1. 将路径替换为绝对的路径(d:/SharedLibrary/hmc-module-static_DEBUG_X64.lib)
// 2. 在vs属性页中配置 VC++目录 ->常规->库目录 ($(静态模块位置)/SharedLibrary)
// 3. 在vs属性页中配置 链接器->常规->附加库目录($(静态模块位置)/SharedLibrary)
#pragma comment(lib, "hmc-module-static_DEBUG_X64.lib")
#else  
#pragma comment(lib, "hmc-module-static_DEBUG_X86.lib")
#endif
#else 
#ifdef _WIN64
#pragma comment(lib, "hmc-module-static_RELEASE_X64.lib")
#else 
#pragma comment(lib, "hmc-module-static_RELEASE_X86.lib")
#endif
#endif

using Menu = hmc_tray_util::Menu;

auto hmc_tray = hmc_tray_util();

static void KK() {
    auto hmc_tray2 = new hmc_tray_util();
    hmc_tray2->start();

    hmc_tray2->on(L"*", [](hmc_tray_util* tray, hmc_tray_util::Event EventID, const std::wstring& envName, hmc_tray_util::Menu* menu) {

        if (EventID == hmc_tray_util::Event::move) {
            return;
        }

        tray->setTrayIconByIco(L"F:\\Root\\icon\\ICON1_1.ico");

        std::wcout << "tray  [ " << tray->id() << " ]   @<  " << envName << "  >  menu->   " << (menu != nullptr ? menu->id : L"[not]") << "\n";

        });
}

static void Tray() {
    KK();

    // 启动
    hmc_tray.start();
    // 设置图标
    hmc_tray.setTrayIconByExtract(L"C:\\Windows\\explorer.exe");


    // 设置主按钮
    hmc_tray.insertMenu(L"显示窗口", L"btn::show_window");
    hmc_tray.insertRadio(L"自动同步", L"btn::auto_sync", true);
    hmc_tray.insertSeparator(L"btn::separator::01");
    hmc_tray.insertMenu(L"马上同步", L"btn::show_start");
    hmc_tray.insertCheck(L"自动启动", L"btn::auto_start_app", true);
    hmc_tray.insertMenu(L"退出程序", L"btn::quit_app");

    // 子键创建方式 演示
    hmc_tray.insertMenu(L"同步方式", L"sub::type_sync", true);

    hmc_tray.insertCheck(L"立刻同步", L"sub::at_once_sync", true);
    // 分割线
    hmc_tray.insertSeparator(L"sub::separator::02");

    hmc_tray.insertCheck(L"停止同步", L"sub::check_stop_sync", true);
    hmc_tray.insertCheck(L"每分钟制", L"sub::check_min", true);
    hmc_tray.insertCheck(L"每小时制", L"sub::check_hourly", true);

    // hmc_tray.on<id>
    /**
     * 他还支持
     * - middleClick
     * - move
     * - click
     * - mouseMove
     * - leftButtonDoubleClick
     * - dblclick
     * - leftButtonUp
     * - leftButtonDown
     * - rightButtonUp
     * - rightButtonDown
     */
    hmc_tray.on(L"*", [](hmc_tray_util* tray, hmc_tray_util::Event EventID, const std::wstring& envName, hmc_tray_util::Menu* menu) {

        if (EventID == hmc_tray_util::Event::clickBtn) {

            std::wcout << "tray  [ " << tray->id() << " ]   @<  " << envName << "  >  menu->   " << (menu != nullptr ? menu->id : L"[not]") << "\n";

            
            if (menu->theIcon != nullptr) {
                menu->removeIcon();
            }
            else {
                menu->setIconByIcon(L"F:\\Root\\icon\\ps_cc_folder.ico");
            }


            return;
        }

        //while (true) {
            tray->setTrayIconByIco(L"F:\\工具\\安装包\\Abobe\\@vposyAdobe_Photoshop_2020_21.2.9.67_ACR13.3_SP_20210608\\Adobe Photoshop 2020 SP\\products\\PHSP\\AdobePhotoshop21-Core_x64\\1\\Application\\Required\\UXP\\com.adobe.ccx.start\\images\\thumbs\\PPRO_prproj.ico");
        //}

        std::wcout << "tray  [ " << tray->id() << " ]   @<  " << envName << "  >  menu->   " << (menu != nullptr ? menu->id : L"[not]") << "\n";

        });



    const std::vector<std::wstring> keyList = {

        // 主按钮是此ID
        
        // 子按钮是这些
        L"sub::at_once_sync", L"sub::separator::02", L"sub::check_stop_sync", L"sub::check_min", L"sub::check_hourly" 
    };


    // 将这些按钮添加到XXX按钮中
    hmc_tray.setMenuItmetoSubMenu(L"sub::type_sync", keyList);
    hmc_tray.setUniqueSelect(keyList);

    Sleep(3000);
    hmc_tray.setTrayInfo(L"时间已经过去了三秒");

    // 禁用此id的按钮：
    hmc_tray.setMenuItmeEnable(L"sub::check_min", true);

    // 隐藏此id的按钮：
    hmc_tray.setMenuItmeVisible(L"btn::quit_app", false);

    Sleep(5000);
    // 设置此按钮的名称为：
    hmc_tray.setMenuItmeName(L"btn::auto_sync", L"取消同步");
    hmc_tray.setTrayInfo(L"正在同步中....");
    Sleep(9000);

    hmc_tray.setMenuItmeName(L"btn::auto_sync", L"自动同步");
    hmc_tray.setTrayInfo(L"上传一次更新为9秒前\n共更新了9999条数据");

    while (true)
    {
        Sleep(5000);
    }

    hmc_tray.close();
}

int main() {

    Tray();

    HWND win = (HWND)4071132;

    auto Status = hmc_windows_util::getWindowHwndStatus(win);
    
    if (Status.index()==0)
    {
        auto& status = std::get<0>(Status);
       
       //std::cout <<"win -> [ " << ULONG(win) << " ] " << "\n";
       
       hmc_console_util::start(true,true,true);

       auto title = hmc_windows_util::get_variant(hmc_windows_util::getWindowTitle(win));
       auto className = hmc_windows_util::get_variant(hmc_windows_util::getClassName(win));

       std::wcout << "title -> [ " << title << " ] " << "\n";
       std::wcout << "className -> [ " << className << " ] " << "\n";
       
       hmc_console_util::stop(true, true, true);

       std::cout << "sub -> [ " ;


       for (size_t i = 0; i < status.sub.size(); i++)
       {
           auto &at = status.sub.at(i);
           //std::cout << ULONG(at) << " , ";
       }
       
       std::cout << " ] " << "\n";

       //std::cout << "parent -> [ " << ULONG(status.parent) << " ] " << "\n";
       //std::cout << "root -> [ " << ULONG(status.root) << " ] " << "\n";
       //std::cout << "next -> [ " << ULONG(status.next) << " ] " << "\n";
       //std::cout << "prev -> [ " << ULONG(status.prev) << " ] " << "\n";
       //std::cout << "end -> [ " << ULONG(status.end) << " ] " << "\n";
       //std::cout << "pid -> [ " << ULONG(status.pid) << " ] " << "\n";
       //std::cout << "threadId -> [ " << ULONG(status.threadId) << " ] " << "\n";
       //std::cout << "owner -> [ " << ULONG(status.owner) << " ] " << "\n";
       //std::cout << "firstChild -> [ " << ULONG(status.firstChild) << " ] " << "\n";
       //std::cout << "firstBrother -> [ " << ULONG(status.firstBrother) << " ] " << "\n";
       //std::cout << "lastSibling -> [ " << ULONG(status.lastSibling) << " ] " << "\n";
       //std::cout << "exists -> [ " << std::boolalpha << status.exists << " ] " << "\n";


       auto is_setCenter = hmc_windows_util::setWindowCenter(status.hwnd);

       std::cout << "is_setCenter -> [ " << std::boolalpha << hmc_windows_util::get_variant(is_setCenter) << " ] " << "\n";


       return 0;
    }


    return 0;
}
