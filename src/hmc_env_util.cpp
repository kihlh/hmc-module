#include <hmc_env_util.h>
#include <hmc_string_util.h>
#include <hmc_process_util.h>
#include <hmc_registr_util.h>
#include <process.h>
#include <Tlhelp32.h>
#include <ShlObj.h>
#include <processenv.h>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#include <set>
#include <unordered_set>

namespace SENV = hmc_env_util::systemEnv;

namespace hmc_env_util
{
    std::map<std::wstring, HANDLE> GLOBAL_ALL_MUTEX_HANDLES;

    namespace systemEnv
    {
        // 用户变量 注册表位置
        const std::wstring userPath = L"Environment";
        HKEY userHkey = HKEY_CURRENT_USER;

        // 用户变量 注册表位置
        const std::wstring systmPath = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
        HKEY systmHkey = HKEY_LOCAL_MACHINE;

        std::vector<std::wstring> freezeEnvKeys;
    }

}

void SENV::update_freezeEnvKeys()
{

    if (SENV::freezeEnvKeys.size() > 10)
    {
        return;
    }

    const std::vector<std::wstring> freezeEnvKeys = {L"CommonProgramFiles(x86)", L"PUBLIC", L"SystemDrive", L"ProgramFiles(x86)", L"ProgramData", L"COMPUTERNAME", L"ALLUSERSPROFILE", L"HOMEDRIVE", L"LOGONSERVER", L"USERDOMAIN", L"APPDATA", L"LOCALAPPDATA", L"HOMEPATH", L"SESSIONNAME", L"USERPROFILE", L"USERNAME", L"CommonProgramFiles", L"ProgramFiles", L"ProgramW6432", L"SystemRoot"};

    for (auto &at : freezeEnvKeys)
    {
        SENV::freezeEnvKeys.push_back(at);
    }
}

bool hmc_env_util::getenv(const std::wstring &name, std::wstring &resultPtr)
{
    bool result = false;

    size_t size;

    ::_wgetenv_s(&size, NULL, 0, name.c_str());

    if (size > 0)
    {
        std::vector<wchar_t> tmpvar(size);

        errno_t state = ::_wgetenv_s(&size, tmpvar.data(), size, name.c_str());

        if (state == 0)
        {
            for (size_t i = 0; i < tmpvar.size(); i++)
            {
                if (tmpvar.size() > 1 && tmpvar.back() == '\0')
                {
                    tmpvar.pop_back();
                }
            }

            resultPtr.append(tmpvar.begin(), tmpvar.end());
            result = true;
        }

        return result;
    }

    return result;
}

std::unordered_map<std::wstring, std::wstring> hmc_env_util::getEnvList()
{

    std::unordered_map<std::wstring, std::wstring> result;

    LPWCH lpvEnv = ::GetEnvironmentStringsW();

    if (lpvEnv != nullptr)
    {

        size_t len = 0;

        while (true)
        {
            len++;
            if (lpvEnv[len] == '\0' && lpvEnv[len + 1] == '\0')
            {
                break;
            }
        }

        auto temp = std::wstring(lpvEnv, lpvEnv + len);
        auto MultiStringArray = hmc_string_util::convertMultiStringArray(temp);
        temp.clear();
        temp.shrink_to_fit();

        ::FreeEnvironmentStringsW(lpvEnv);

        for (size_t i = 0; i < MultiStringArray.size(); i++)
        {
            auto &at = MultiStringArray.at(i);

            size_t sep = at.rfind('=');

            if (sep > at.size())
            {
                sep = at.size();
            }

            std::wstring key = at.substr(0, sep);

            std::wstring value = sep + 1 >= at.size() ? std::wstring() : at.substr(sep + 1, at.size());

            result.insert(std::pair<std::wstring, std::wstring>(key, value));
        }
    }

    return result;
}

std::vector<std::wstring> hmc_env_util::getcmd()
{

    std::vector<std::wstring> cmdList;

    int size = 0;

    LPWSTR *cmd_arg_list = ::CommandLineToArgvW(::GetCommandLineW(), &size);

    if (!size || cmd_arg_list == nullptr)
        return cmdList;

    for (size_t i = 1; i < static_cast<size_t>(size); ++i)
    {
        wchar_t *arg = cmd_arg_list[i];
        cmdList.push_back(arg);
    }

    ::HeapFree(::GetProcessHeap(), 0, (cmd_arg_list));

    return cmdList;
}

std::wstring hmc_env_util::getcmdLine()
{
    return std::wstring(::GetCommandLineW());
}

bool hmc_env_util::setcwd(const std::wstring &DirPath)
{
    return ::SetCurrentDirectoryW(DirPath.c_str());
}

std::uint64_t hmc_env_util::getpid()
{
    DWORD processId = ::GetCurrentProcessId();
    return processId >= 0 ? static_cast<std::uint64_t>(processId) : 0;
}

std::uint64_t hmc_env_util::getppid()
{
    std::uint64_t ppid = 0;

    DWORD processId = ::GetCurrentProcessId();

    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (::Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (pe32.th32ProcessID == processId)
            {
                ::CloseHandle(hSnapshot);
                ppid = static_cast<std::uint64_t>(pe32.th32ParentProcessID);
                return ppid;
            }

        } while (::Process32Next(hSnapshot, &pe32));
    }

    ::CloseHandle(hSnapshot);
    return ppid;
}

std::wstring hmc_env_util::getCwd()
{

    std::wstring result;
    result.resize(MAX_PATH);

    DWORD len = 0;

    if (len = ::GetCurrentDirectoryW(MAX_PATH, &result[0]))
    {
        result.resize(len);
    }

    return result;
}

std::wstring hmc_env_util::getExecDirPath()
{
    auto pid = getpid();
    std::wstring temp = hmc_process_util::getProcessFilePath(static_cast<DWORD>(pid), false, false);
    for (size_t i = 0; i < temp.size(); i++)
    {
        if (temp.empty())
        {
            break;
        }

        auto at = temp.back();

        if (at == '\\' || at == '/')
        {
            break;
        }

        temp.pop_back();
    }

    return temp;
}

std::wstring hmc_env_util::escapeEnvVariable(const std::wstring input)
{
    std::wstring result(input.begin(), input.end());

    uint32_t bufLength = ::ExpandEnvironmentStringsW(input.c_str(), nullptr, 0);

    result.resize(static_cast<size_t>(bufLength) + MAX_PATH);

    bufLength = ::ExpandEnvironmentStringsW(input.c_str(), &result[0], static_cast<DWORD>(result.size()));

    result.resize(bufLength);

    if (!result.empty() && result.back() == '\0')
    {
        result.pop_back();
    }

    result.shrink_to_fit();

    return result;
}

// 判断 x64 系统
static BOOL isSystemFor64bit()
{
    SYSTEM_INFO SystemInfo;
    ::GetNativeSystemInfo(&SystemInfo);
    if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return TRUE;
    else
        return FALSE;
}

std::wstring hmc_env_util::getArch()
{
    return (isSystemFor64bit() ? L"x64" : L"x32");
}

bool hmc_env_util::removeEnv(const std::wstring key)
{
    bool result = false;
    result = ::SetEnvironmentVariableW(key.c_str(), NULL);
    return result;
}

bool hmc_env_util::hasEnvKey(const std::wstring key)
{
    bool result = false;

    DWORD size = ::GetEnvironmentVariableW(key.c_str(), NULL, 0);

    if (size == 0)
    {
        return result;
    }

    result = true;
    return result;
}

bool hmc_env_util::putenv(const std::wstring key, const std::wstring Value)
{
    bool result = false;

    result = ::SetEnvironmentVariableW(key.c_str(), Value.c_str());

    return result;
}

bool hmc_env_util::putenv(const std::wstring key, const std::wstring Value, const bool append)
{
    bool result = false;
    std::wstring newValue;

    if (append)
    {
        if (!getenv(key, newValue))
        {
            return false;
        };

        newValue.push_back(';');
        newValue.append(Value);

        result = ::SetEnvironmentVariableW(key.c_str(), newValue.c_str());
        return result;
    }

    return result;
}

bool hmc_env_util::putenv(const std::wstring key, const std::vector<std::wstring> Value)
{
    bool result = false;

    std::wstring ValueStr;

    for (auto &&data : Value)
    {
        ValueStr.append(data);
        ValueStr.push_back(';');
    }

    if (!ValueStr.empty() && ValueStr.back() == ';')
    {
        ValueStr.pop_back();
    }

    result = ::SetEnvironmentVariableW(key.c_str(), ValueStr.c_str());

    return result;
}

bool hmc_env_util::Mutex::create(const std::wstring MutexName)
{

    bool result = false;

    HANDLE hMutex = ::CreateMutexW(NULL, FALSE, MutexName.c_str());

    if (hMutex != NULL)
    {
        result = true;
        GLOBAL_ALL_MUTEX_HANDLES.insert(std::pair<std::wstring, HANDLE>(MutexName, hMutex));
    }

    return result;
}

bool hmc_env_util::Mutex::has(const std::wstring MutexName)
{

    bool result = false;

    if (GLOBAL_ALL_MUTEX_HANDLES.find(MutexName) != GLOBAL_ALL_MUTEX_HANDLES.end())
    {
        HANDLE hMutex = GLOBAL_ALL_MUTEX_HANDLES[MutexName];
        result = hMutex != nullptr;
        if (!result)
        {
            GLOBAL_ALL_MUTEX_HANDLES.erase(MutexName);
        }
    }

    return result;
}

bool hmc_env_util::Mutex::remove(const std::wstring MutexName)
{

    bool result = false;

    if (GLOBAL_ALL_MUTEX_HANDLES.find(MutexName) != GLOBAL_ALL_MUTEX_HANDLES.end())
    {
        HANDLE hMutex = GLOBAL_ALL_MUTEX_HANDLES[MutexName];
        result = ::CloseHandle(hMutex);
        GLOBAL_ALL_MUTEX_HANDLES.erase(MutexName);
    }
    return result;
}

std::vector<std::wstring> hmc_env_util::Mutex::list()
{

    std::vector<std::wstring> result;

    for (auto &at : GLOBAL_ALL_MUTEX_HANDLES)
    {
        result.push_back(at.first);
    }

    return result;
}

bool hmc_env_util::systemEnv::keyDiff(const std::wstring _key, const std::wstring _key2)
{
    auto key1 = hmc_string_util::text_to_upper(_key);
    auto key2 = hmc_string_util::text_to_upper(_key2);

    const auto temp = std::vector<wchar_t>({'\0', '\n', '\r'});

    hmc_string_util::removeMatchChar(&key1, temp);
    hmc_string_util::removeMatchChar(&key2, temp);

    if (key1.empty() || key2.empty())
    {
        return false;
    }

    return key1 == key2;
}

std::vector<std::wstring> hmc_env_util::systemEnv::getSyskeyList()
{

    std::vector<std::wstring> result;

    std::set<std::wstring> temp_list;

    update_freezeEnvKeys();
    for (auto key : freezeEnvKeys)
    {
        temp_list.insert(key);
    }

    auto Keys = hmc_registr_util::queryRegistrFolderInfo(userHkey, userPath, false, true);

    for (auto key : Keys.valueKeyList)
    {
        temp_list.insert(key.name);
    }

    result.assign(temp_list.begin(), temp_list.end());

    return result;
}

std::vector<std::wstring> hmc_env_util::systemEnv::getUsekeyList()
{

    std::vector<std::wstring> result;

    std::set<std::wstring> temp_list;

    update_freezeEnvKeys();
    for (auto key : freezeEnvKeys)
    {
        temp_list.insert(key);
    }

    auto Keys = hmc_registr_util::queryRegistrFolderInfo(systmHkey, systmPath, false, true);

    for (auto key : Keys.valueKeyList)
    {
        temp_list.insert(key.name);
    }

    result.assign(temp_list.begin(), temp_list.end());

    return result;
}

void hmc_env_util::systemEnv::getAllkeyList(std::vector<std::wstring> &SyskeyList, std::vector<std::wstring> &UsekeyList)
{

    std::set<std::wstring> temp_list;

    update_freezeEnvKeys();
    for (auto key : freezeEnvKeys)
    {
        temp_list.insert(key);
        SyskeyList.push_back(key);
        UsekeyList.push_back(key);
    }

    {
        auto Keys = hmc_registr_util::queryRegistrFolderInfo(systmHkey, systmPath, false, true);

        for (auto key : Keys.valueKeyList)
        {
            temp_list.insert(key.name);
        }

        SyskeyList.assign(temp_list.begin(), temp_list.end());
        temp_list.clear();
        for (auto key : freezeEnvKeys)
        {
            temp_list.insert(key);
        }
    }

    auto Keys = hmc_registr_util::queryRegistrFolderInfo(userHkey, userPath, false, true);

    for (auto key : Keys.valueKeyList)
    {
        temp_list.insert(key.name);
    }

    UsekeyList.assign(temp_list.begin(), temp_list.end());
}

bool hmc_env_util::systemEnv::hasSysKeyExists(const std::wstring key)
{
    bool result = hasFreezeKey(key);

    if (result)
    {
        return result;
    }

    auto temp_2 = getSyskeyList();

    for (auto _key : temp_2)
    {

        if (keyDiff(_key, key))
        {
            return true;
        }
    }

    return result;
}

bool hmc_env_util::hasFreezeKey(const std::wstring key)
{

    bool result = false;
    hmc_env_util::systemEnv::update_freezeEnvKeys();

    for (auto _key : hmc_env_util::systemEnv::freezeEnvKeys)
    {

        if (hmc_env_util::systemEnv::keyDiff(_key, key))
        {
            return true;
        }
    }

    return result;
}

bool hmc_env_util::systemEnv::hasUseKeyExists(const std::wstring key)
{

    bool result = hasFreezeKey(key);

    if (result)
    {
        return result;
    }

    auto temp_2 = getUsekeyList();

    for (auto _key : temp_2)
    {

        if (keyDiff(_key, key))
        {
            return true;
        }
    }

    return result;
}

bool hmc_env_util::systemEnv::removeUse(const std::wstring key)
{
    bool result = false;

    if (hasFreezeKey(key))
    {
        return result;
    }

    hmc_registr_util::removeRegistrKey(userHkey, userPath, key);

    result = !hmc_registr_util::hasRegistrKey(userHkey, userPath, key);
    return result;
}

bool hmc_env_util::systemEnv::removeSys(const std::wstring key)
{
    bool result = false;

    if (hasFreezeKey(key))
    {
        return result;
    }

    hmc_registr_util::removeRegistrKey(systmHkey, systmPath, key);

    result = !hmc_registr_util::hasRegistrKey(systmHkey, systmPath, key);
    return result;
}

bool hmc_env_util::systemEnv::removeAll(const std::wstring key)
{
    bool result = false;

    if (hasFreezeKey(key))
    {
        return result;
    }

    hmc_registr_util::removeRegistrKey(systmHkey, systmPath, key);
    hmc_registr_util::removeRegistrKey(userHkey, userPath, key);

    result =
        !hmc_registr_util::hasRegistrKey(systmHkey, systmPath, key) &&
        !hmc_registr_util::hasRegistrKey(userHkey, userPath, key);

    return result;
}

bool hmc_env_util::systemEnv::hasExpval(std::wstring key)
{
    bool result = false;
    hmc_registr_util::chValueStat userValueStat = hmc_registr_util::getValueStat(userHkey, userPath, key);
    if (userValueStat.exists)
    {
        result = userValueStat.type == REG_EXPAND_SZ;
        return result;
    }

    hmc_registr_util::chValueStat sysValueStat = hmc_registr_util::getValueStat(systmHkey, systmPath, key);
    if (sysValueStat.exists)
    {
        result = sysValueStat.type == REG_EXPAND_SZ;
        return result;
    }
    return result;
}

bool hmc_env_util::systemEnv::hasKeyExists(const std::wstring key)
{
    return hmc_env_util::systemEnv::hasSysKeyExists(key) ? true : hmc_env_util::systemEnv::hasUseKeyExists(key);
}

bool hmc_env_util::systemEnv::getPath(std::wstring &pEnvStr, bool transMean)
{
    bool result = false;

    auto list = std::vector<std::wstring>();

    const std::wstring PATH_KEY = {'P', 'A', 'T', 'H'};

    if (hmc_env_util::systemEnv::getList(PATH_KEY, list))
    {
        hmc_string_util::join(list, L";", pEnvStr);
        if (transMean)
        {
            readEnvCompletion(PATH_KEY, pEnvStr, false);
        }

        result = true;
        return result;
    }

    return result;
}

bool hmc_env_util::systemEnv::getList(const std::wstring key, std::vector<std::wstring> &pEnvStrList)
{

    bool result = false;

    std::wstring pEnvStr;

    if (hmc_env_util::hasFreezeKey(key))
    {
        result = hmc_env_util::getenv(key, pEnvStr);
        auto temp = hmc_string_util::split(pEnvStr, L';', true);
        pEnvStrList.assign(temp.begin(), temp.end());
        return result;
    }

    if (auto openSysEnv = hmc_registr_util::OpenKeyToken(systmHkey, systmPath, false))
    {
        auto temp = hmc_registr_util::GetRegistrString(openSysEnv, key);
        pEnvStr.append(std::move(temp));
    }

    if (auto openUserEnv = hmc_registr_util::OpenKeyToken(userHkey, userPath, false))
    {
        pEnvStr.push_back(';');
        auto temp = hmc_registr_util::GetRegistrString(openUserEnv, key);
        pEnvStr.append(std::move(temp));
    }

    auto temp = hmc_string_util::split(pEnvStr, L';', true);

    hmc_string_util::removeDuplicates(&temp);

    pEnvStrList.assign(temp.begin(), temp.end());

    return (result = pEnvStrList.size() != 0);
}

bool hmc_env_util::systemEnv::readEnvCompletion(const std::wstring _key, std::wstring &pEnvStr, const bool useLocal)
{

    bool result = false;
    size_t transMeanLoopLen = 0;

    pEnvStr.reserve(pEnvStr.size() + 4096);

    std::vector<std::wstring> key_list = hmc_string_util::findKeyList(pEnvStr, '%');

    if (key_list.empty())
    {
        result = true;
        return result;
    }

    auto strTempServe = std::wstring();

    strTempServe.reserve(4096);

    auto openSysToken = hmc_registr_util::OpenKeyToken(systmHkey, systmPath, false);
    auto openUseToken = hmc_registr_util::OpenKeyToken(userHkey, userPath, false);

    do
    {
        key_list = hmc_string_util::findKeyList(pEnvStr, '%');

        if (key_list.empty())
        {
            result = true;
            return result;
        }

        // 排除掉所有不规范的key
        size_t length = key_list.size();
        for (size_t index = length; ((index-- > 0) && !(index >= length));)
        {
            auto at = &key_list.at(index);
            if ((at->empty() || hmc_env_util::systemEnv::keyDiff(*at, _key)) && index < length)
            {
                key_list.erase(key_list.begin() + index);
            }
        }

        if (key_list.empty())
        {
            break;
        }

        for (size_t i = 0; i < key_list.size(); i++)
        {
            auto key = key_list.at(i);
            if (key.empty() || hmc_env_util::systemEnv::keyDiff(key, _key))
                continue;

            key.reserve(key.size() + 2);
            strTempServe.clear();

            if (hmc_env_util::hasFreezeKey(key))
            {
                hmc_env_util::getenv(key, strTempServe);
                hmc_string_util::joinKey(&key, '%');
                hmc_string_util::replace(pEnvStr, key, strTempServe);
                continue;
            }

            std::wstring temp1 = hmc_registr_util::GetRegistrString(openUseToken.token, key, false);
            strTempServe.append(std::move(temp1));

            if (!strTempServe.empty())
            {
                hmc_string_util::joinKey(&key, '%');
                hmc_string_util::replace(pEnvStr, key, strTempServe);
                continue;
            }

            strTempServe.clear();

            std::wstring temp2 = hmc_registr_util::GetRegistrString(openSysToken.token, key, false);

            strTempServe.append(std::move(temp2));

            if (!strTempServe.empty())
            {
                hmc_string_util::joinKey(&key, '%');
                hmc_string_util::replace(pEnvStr, key, strTempServe);
                continue;
            }

            if (useLocal)
            {
                hmc_env_util::getenv(key, strTempServe);

                if (!strTempServe.empty())
                {
                    hmc_string_util::joinKey(&key, '%');
                    hmc_string_util::replace(pEnvStr, key, strTempServe);
                    continue;
                }
            }

            strTempServe.clear();
        }

        result = true;

    } while ((transMeanLoopLen++) >= 50);

    // 收缩内存
    if (pEnvStr.capacity() - pEnvStr.size() > 50)
    {
        auto theSize = pEnvStr.size();
        pEnvStr.insert(theSize, 10, '\0');
        pEnvStr.shrink_to_fit();
        pEnvStr.resize(theSize);
    }

    return result;
}

bool hmc_env_util::systemEnv::getUse(const std::wstring key, std::wstring &pVariable, bool transMean)
{

    bool result = false;

    if (hmc_env_util::hasFreezeKey(key))
    {
        result = hmc_env_util::getenv(key, pVariable);
        return result;
    }

    pVariable.clear();

    if (keyDiff(key, L"PATH"))
    {
        result = hmc_env_util::systemEnv::getPath(pVariable, transMean);

        if (!result)
        {
            result = hmc_env_util::getenv(key, pVariable);
        }

        return result;
    }

    if (auto openEnv = hmc_registr_util::OpenKeyToken(userHkey, userPath, false))
    {
        std::wstring temp = hmc_registr_util::GetRegistrString(openEnv.token, key);
        temp.shrink_to_fit();
        pVariable.reserve(temp.size() + 5);
        pVariable.clear();
        pVariable.append(std::move(temp));
        result = !pVariable.empty();

        if (transMean && openEnv.getValueStat(key).type != REG_EXPAND_SZ)
        {
            return result;
        }
    }

    if (transMean && hmc_string_util::includes(pVariable, '%'))
    {
        auto KeyList = hmc_string_util::findKeyList(pVariable, L'%');
        if (KeyList.empty())
        {
            return result;
        }
        readEnvCompletion(key, pVariable);
    }

    return result;
}

bool hmc_env_util::systemEnv::getSys(const std::wstring key, std::wstring &pVariable, bool transMean)
{

    bool result = false;

    if (hmc_env_util::hasFreezeKey(key))
    {
        result = hmc_env_util::getenv(key, pVariable);
        return result;
    }

    pVariable.clear();

    if (keyDiff(key, L"PATH"))
    {
        result = hmc_env_util::systemEnv::getPath(pVariable, transMean);

        if (!result)
        {
            result = hmc_env_util::getenv(key, pVariable);
        }

        return result;
    }

    if (auto openEnv = hmc_registr_util::OpenKeyToken(systmHkey, systmPath, false))
    {
        std::wstring temp = hmc_registr_util::GetRegistrString(openEnv.token, key);
        temp.shrink_to_fit();
        pVariable.reserve(temp.size() + 5);
        pVariable.clear();
        pVariable.append(std::move(temp));
        result = !pVariable.empty();

        if (transMean && openEnv.getValueStat(key).type != REG_EXPAND_SZ)
        {
            return result;
        }
    }

    if (transMean && hmc_string_util::includes(pVariable, '%'))
    {
        auto KeyList = hmc_string_util::findKeyList(pVariable, L'%');
        if (KeyList.empty())
        {
            return result;
        }
        readEnvCompletion(key, pVariable);
    }

    return result;
}

bool hmc_env_util::systemEnv::get(const std::wstring key, std::wstring &pEnvStr, bool transMean)
{
    bool result = false;

    if ((result = getUse(key, pEnvStr, transMean)))
    {
        return result;
    }

    if ((result = getSys(key, pEnvStr, transMean)))
    {
        return result;
    }

    return result;
}

std::wstring hmc_env_util::systemEnv::escapeEnvVariable(const std::wstring input)
{
    std::wstring result(input.begin(), input.end());

    if (!result.empty() && result.back() == '\0')
    {
        result.pop_back();
    }

    result.shrink_to_fit();

    return result;
}

std::vector<hmc_env_util::systemEnv::chFormatVariableData> hmc_env_util::systemEnv::GetGlobalVariableAll(const bool systemEnv, const bool userEnv)
{
    std::vector<hmc_env_util::systemEnv::chFormatVariableData> result;

    hmc_env_util::systemEnv::update_freezeEnvKeys();

    for (auto _key : hmc_env_util::systemEnv::freezeEnvKeys)
    {
        auto data = chFormatVariableData();
        data.name = _key;

        hmc_env_util::getenv(_key, data.data);

        data.size = data.data.size();
        data.type_user = true;
        data.escape = false;
        data.upper = hmc_string_util::text_to_upper(_key);
        result.emplace_back(data);
    }

    if (userEnv)
    {

        auto openEnv = hmc_registr_util::OpenKeyToken(userHkey, userPath, false);

        if (openEnv)
        {
            auto folderStat = openEnv.queryFolderStatW(false, true);

            for (size_t i = 0; i < folderStat.valueKeyList.size(); i++)
            {
                auto valueKey = folderStat.valueKeyList.at(i);

                if (hasFreezeKey(valueKey.name))
                {
                    continue;
                }

                auto temp = hmc_registr_util::GetRegistrString(openEnv.token, valueKey.name);

                auto data = chFormatVariableData();
                data.name = valueKey.name;
                data.data = std::move(temp);
                data.type_user = true;
                data.size = valueKey.size;
                data.escape = valueKey.type == REG_EXPAND_SZ;
                result.emplace_back(data);
            }
        }
    }

    if (systemEnv)
    {

        auto openEnv = hmc_registr_util::OpenKeyToken(systmHkey, systmPath, false);

        if (openEnv)
        {
            auto folderStat = openEnv.queryFolderStatW(false, true);

            for (size_t i = 0; i < folderStat.valueKeyList.size(); i++)
            {
                auto valueKey = folderStat.valueKeyList.at(i);

                if (hasFreezeKey(valueKey.name))
                {
                    continue;
                }

                auto temp = hmc_registr_util::GetRegistrString(openEnv.token, valueKey.name);

                auto data = chFormatVariableData();
                data.name = valueKey.name;
                data.data = std::move(temp);
                data.type_user = false;
                data.size = valueKey.size;
                data.escape = valueKey.type == REG_EXPAND_SZ;
                result.emplace_back(data);
            }
        }
    }

    return result;
}

std::map<std::wstring, std::wstring> hmc_env_util::systemEnv::getGlobalVariable(const bool transMean)
{

    std::map<std::wstring, std::wstring> result;

    std::unordered_set<std::wstring> TRANS_MEAN_LIST;

    auto list = hmc_env_util::systemEnv::GetGlobalVariableAll(true, true);

    const std::wstring PATH_KEY = {'P', 'A', 'T', 'H'};

    auto strTempServe = std::wstring();

    if (transMean)
    {
        strTempServe.reserve(1024 * 10);
    }

    for (auto item : list)
    {
        auto KEY = hmc_string_util::text_to_upper(item.name);

        if (keyDiff(KEY, PATH_KEY) && result.find(item.name) == result.end())
        {
            result.insert(std::make_pair(KEY, item.data));
            auto at = result.at(KEY);
            hmc_env_util::systemEnv::getPath(at, false);
            TRANS_MEAN_LIST.insert(KEY);
            continue;
        }

        if (result.find(KEY) != result.end())
        {
            continue;
        }

        if (transMean && item.escape)
        {

            strTempServe.append(item.data.begin(), item.data.end());
            auto KeyList = hmc_string_util::findKeyList(strTempServe, L'%');

            for (auto _key : KeyList)
            {
                auto key = hmc_string_util::text_to_upper(_key);

                if (result.find(key) != result.end())
                {
                    auto temp = hmc_string_util::joinKey(_key, '%');
                    hmc_string_util::replace(strTempServe, temp, result.at(key));
                }
            }

            result[KEY] = std::wstring(strTempServe.begin(), strTempServe.end());

            strTempServe.clear();

            if (KeyList.size() > 0)
            {
                TRANS_MEAN_LIST.insert(KEY);
            }

            continue;
        }

        result[KEY] = item.data;
    }

    if (transMean)
    {
        size_t index_loop_max_len = TRANS_MEAN_LIST.size() * 2;

        for (size_t index = 0; index < 2; index++)
        {
            for (auto KEY : TRANS_MEAN_LIST)
            {

                if (KEY.empty() || result.find(KEY) == result.end())
                {
                    continue;
                }

                strTempServe.append(result.at(KEY).begin(), result.at(KEY).end());
                auto KeyList = hmc_string_util::findKeyList(strTempServe, L'%');
                size_t len = KeyList.size();

                for (auto _key : KeyList)
                {
                    auto key = hmc_string_util::text_to_upper(_key);

                    if (result.find(key) != result.end())
                    {
                        auto temp = hmc_string_util::joinKey(_key, '%');
                        hmc_string_util::replace(strTempServe, temp, result.at(key));
                        len--;
                    }
                }

                auto at = &result[KEY];
                at->resize(strTempServe.size());
                at->replace(0, at->size(), strTempServe);
                strTempServe.clear();

                // 此变量里面的变量转义完成了 删除元素并跳出这次循环
                if (len == 0)
                {
                    auto it = TRANS_MEAN_LIST.find(KEY);
                    TRANS_MEAN_LIST.erase(it);
                    if (index > 0)
                    {
                        index--;
                    }
                    break;
                }
            }

            // 防止出现循环调用 例如 p1=%p2%  p2=%p1%\\123\\456%
            if ((index_loop_max_len--) <= 1)
            {
                return result;
            }
        }
    }

    return result;
}

void hmc_env_util::systemEnv::updateThis(bool remove, bool update, std::vector<std::wstring> *filterList)
{
    auto env_list = hmc_env_util::systemEnv::getGlobalVariable(true);

    if (env_list.empty())
    {
        return;
    }

    for (auto &&at : env_list)
    {
        const std::wstring &key = at.first;
        std::wstring &value = at.second;
        std::wstring KEY = hmc_string_util::text_to_upper(key);

        bool has_exist = hmc_env_util::hasEnvKey(KEY);

        if (filterList != nullptr)
        {
            bool is_filter = false;
            for (size_t i = 0; i < filterList->size(); i++)
            {
                auto &it = filterList->at(i);

                if (!it.empty() && keyDiff(key, it))
                {
                    is_filter = true;
                    continue;
                }
            }

            if (is_filter)
            {
                continue;
            }
        }

        if (!has_exist)
        {
            hmc_env_util::putenv(key, value);
            continue;
        }

        if (update && has_exist)
        {
            std::wstring valueTempPtr;
            hmc_env_util::getenv(key, valueTempPtr);

            if (valueTempPtr == value)
            {
                continue;
            }

            hmc_env_util::putenv(key, value);
            continue;
        }
    }

    if (remove)
    {
        auto envList = hmc_env_util::getEnvList();

        for (auto &&at : envList)
        {
            const std::wstring &key = at.first;
            std::wstring KEY = hmc_string_util::text_to_upper(key);

            bool has_exist = env_list.find(KEY) != env_list.end();

            if (has_exist)
            {
                continue;
            }

            hmc_env_util::removeEnv(key);
        }
    }
}