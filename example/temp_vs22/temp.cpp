#include <fstream>
#include <string>
#include <windows.h>
#include <hmc_env_util.h>
#include <hmc_string_util.h>
#include <hmc_registr_util.h>

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


int main() {

    auto key_list = hmc_env_util::getEnvList();

    auto key_lisct = hmc_env_util::escapeEnvVariable(L"%%%ddd%%2222%,%HMP%%333%safasgsdg%5,");
    

    const std::wstring systmPath = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    HKEY systmHkey = HKEY_LOCAL_MACHINE;

    auto key = hmc_registr_util::OpenKeyToken(systmHkey, systmPath,L"");

    auto cop = hmc_registr_util::queryRegistrFolderInfoW(key,true,true);

    auto GlobalVariable_List = hmc_env_util::systemEnv::GetGlobalVariableAll(true, true);

    auto GlobalVariable_List2 = hmc_env_util::systemEnv::getGlobalVariable(true);

    std::wstring path_all = std::wstring();


    hmc_env_util::systemEnv::getPath(path_all,true);

    return 0;
}
