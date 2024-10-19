#pragma once

// 防止重复导入
#ifndef MODE_INTERNAL_INCLUDE_HMC_ENV_UTIL_H
#define MODE_INTERNAL_INCLUDE_HMC_ENV_UTIL_H

#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include <optional>
#include <unordered_map>

namespace hmc_env_util
{
    extern std::map<std::wstring, HANDLE> GLOBAL_ALL_MUTEX_HANDLES;

    /**
     * @brief 判断是不是冻结的key
     *
     * @param key
     * @return true
     * @return false
     */
    extern bool hasFreezeKey(const std::wstring key);
    /**
     * @brief 获取环境变量值
     * @param name
     * @param data
     * @return
     */
    extern bool getenv(const std::wstring &name, std::wstring &resultPtr);
    /**
     * @brief 设置当前的工作路径
     * @param DirPath
     * @return
     */
    extern bool setcwd(const std::wstring &DirPath);
    /**
     * @brief 获取命令行
     * @return
     */
    extern std::wstring getcmdLine();
    /**
     * @brief 获取启动命令
     * @return
     */
    extern std::vector<std::wstring> getcmd();
    /**
     * @brief 获取当前进程id
     * @return
     */
    extern std::uint64_t getpid();
    /**
     * @brief 获取主进程id
     * @return
     */
    extern std::uint64_t getppid();
    /**
     * @brief 获取工作目录
     * @return
     */
    extern std::wstring getCwd();
    /**
     * @brief 获取可执行文件所在的文件夹
     * @return
     */
    extern std::wstring getExecDirPath();
    /**
     * @brief 获取当前应用的平台
     * @return
     */
    extern std::wstring getArch();
    /**
     * @brief 判断是否存在此环境变量的键值
     * @param key
     * @return
     */
    extern bool hasEnvKey(const std::wstring key);
    /**
     * @brief 添加环境变量 (请注意这会传递到所有后面启动的子进程)
     * @param key
     * @param Value
     * @return
     */
    extern bool putenv(const std::wstring key, const std::wstring Value);
    /**
     * @brief 添加环境变量 (请注意这会传递到所有后面启动的子进程)
     * @param key
     * @param Value
     * @param append
     * @return
     */
    extern bool putenv(const std::wstring key, const std::wstring Value, const bool append);
    /**
     * @brief 添加环境变量 (请注意这会传递到所有后面启动的子进程)
     * @param key
     * @param Value
     * @return
     */
    extern bool putenv(const std::wstring key, const std::vector<std::wstring> Value);
    /**
     * @brief 获取当前的变量环境列表
     * @return
     */
    extern std::unordered_map<std::wstring, std::wstring> getEnvList();
    /**
     * @brief 删除指定的变量值
     * @param key
     * @return
     */
    extern bool removeEnv(const std::wstring key);
    /**
     * @brief 获取文本并带入环境
     *
     * @param input
     * @return std::wstring
     */
    extern std::wstring escapeEnvVariable(const std::wstring input);

    namespace Mutex
    {
        /**
         * @brief 创建一个互斥体
         * @param MutexName
         * @return
         */
        extern bool create(const std::wstring MutexName);
        /**
         * @brief 判断是否有这个互斥体
         * @param MutexName
         * @return
         */
        extern bool has(const std::wstring MutexName);
        /**
         * @brief 删除通过此方法创建的互斥体
         * @param MutexName
         * @return
         */
        extern bool remove(const std::wstring MutexName);
        /**
         * @brief 获取当前已经创建的互斥体内容
         * @return
         */
        extern std::vector<std::wstring> list();
    }

    /**
     * @brief 系统变量操作（会写入系统和运行环境）
     *
     */
    namespace systemEnv
    {
        struct chFormatVariableData
        {
            // 是否需要转义
            bool escape;
            // 是用户数据
            bool type_user;
            // 数据
            std::wstring data;
            // 名称
            std::wstring name;
            // 大写名
            std::wstring upper;
            // 数据大小
            size_t size;
            chFormatVariableData() : escape(false), type_user(false), size(0) {};
        };

        // 这些值不允许删除也不允许修改
        extern std::vector<std::wstring> freezeEnvKeys;
        // 初始化默认环境变量 冻结值
        void update_freezeEnvKeys();
        /**
         * @brief 对比两个键是否相同 并且忽略大小写以及空白字符
         *
         * @param key 键1
         * @param key2 键2
         * @return true
         * @return false
         */
        extern bool keyDiff(const std::wstring key, const std::wstring key2);
        /**
         * @brief 获取原始的变量列表 不转义 排序-> [ 冻结值  >  用户变量  >  系统变量 ]
         *
         * @return std::vector<chFormatVariableData>
         */
        extern std::vector<chFormatVariableData> GetGlobalVariableAll(const bool systemEnv = true, const bool userEnv = true);
        /**
         * @brief 获取变量列表 转义
         *
         * @return std::map<std::wstring, std::wstring>
         */
        extern std::map<std::wstring, std::wstring> getGlobalVariable(const bool transMean = true);

        /**
         * @brief 尝试从注册表读取并替换 环境变量组
         *
         * @param _key 键 用于防止loop
         * @param pEnvStr 数据写入到此传址string变量
         * @param useLocal 找不到时是否允许从变量环境读取
         * @return true
         * @return false
         */
        extern bool readEnvCompletion(const std::wstring _key, std::wstring &pEnvStr, const bool useLocal = true);

        /**
         * @brief 获取文本并带入环境
         *
         * @param input
         * @return std::wstring
         */
        extern std::wstring escapeEnvVariable(const std::wstring input);
        /**
         * @brief 获取系统 所有变量名 但是不查询值
         *
         * @return std::vector<std::wstring>
         */
        extern std::vector<std::wstring> getSyskeyList();
        /**
         * @brief 获取用户 所有变量名 但是不查询值
         *
         * @return std::vector<std::wstring>
         */
        extern std::vector<std::wstring> getUsekeyList();
        /**
         * @brief 获取系统 所有变量名 但是不查询值
         *
         * @return std::vector<std::wstring>
         */
        extern void getAllkeyList(std::vector<std::wstring> &SyskeyList, std::vector<std::wstring> &UsekeyList);
        /**
         * @brief 写入变量到用户变量
         *
         * @param key 键
         * @param Value 值
         * @param append 添加到尾部 而不是替换
         * - 默认 false
         * @param transMean 是自字符转义
         * - 默认 false
         * @return true
         * @return false
         */
        extern bool putUse(const std::wstring key, const std::wstring Value, bool append = false, bool transMean = false);
        /**
         * @brief 写入变量到系统变量
         *
         * @param key 键
         * @param Value 值
         * @param append 添加到尾部 而不是替换
         * - 默认 false
         * @param transMean 是自字符转义
         * - 默认 false
         * @return true
         * @return false
         */
        extern bool putSys(const std::wstring key, const std::wstring Value, bool append = false, bool transMean = false);
        /**
         * @brief 获取指定键值(从系统)
         *
         * @param key 键
         * @param pEnvStr 数据写入到此传址string变量
         * @param transMean 转义(默认true)
         * @return true
         * @return false
         */
        extern bool getSys(const std::wstring key, std::wstring &pEnvStr, bool transMean = true);

        /**
         * @brief 获取指定键的所有值并将其转为数组
         *
         * @param key 键
         * @param pEnvStr 缓冲区
         * @return true
         * @return false
         */
        extern bool getList(const std::wstring key, std::vector<std::wstring> &pEnvStrList);
        extern bool getPath(std::wstring &pEnvStr, bool transMean = true);
        /**
         * @brief 获取系统环境变量
         *
         * @param key 键
         * @param pEnvStr 数据写入到此传址string变量
         * @param transMean 转义(默认true)
         * @return true
         * @return false
         */
        extern bool getUse(const std::wstring key, std::wstring &pEnvStr, bool transMean = true);
        /**
         * @brief 获取指定键值 按照默认优先级
         * ?- 用户变量和系统变量同时有非数组键  -> 用户变量
         * ?- 用户变量和系统变量同时为数组键    -> 用户变量数组
         * ?- 用户变量为数组键 系统变量为文本键  -> 用户文本键 排除系统
         * ?- 系统变量为文本键 用户变量为数组    -> 用户变量数组 排除系统
         * ?- 系统变量存在 用户变量为空文本      -> 排除此变量
         * ?- PATH                          -> 合并数组
         * @param key 键
         * @param pEnvStr 数据写入到此传址string变量
         * @return true
         * @return false
         */
        extern bool get(const std::wstring key, std::wstring &pEnvStr, bool transMean = true);
        /**
         * @brief 删除用户变量
         *
         * @param key 键
         * @return true
         * @return false
         */
        extern bool removeUse(const std::wstring key);
        /**
         * @brief 删除系统变量
         * ?部分key 如 SystemDeive 是自动忽略的
         * @param key 键
         * @return true
         * @return false
         */
        extern bool removeSys(const std::wstring key);
        /**
         * @brief 删除所有此变量
         * ?部分key 如 SystemDeive 是自动忽略的
         * @param key 键
         * @return true
         * @return false
         */
        extern bool removeAll(const std::wstring key);
        /**
         * @brief 从注册表读取并更新到当前环境
         *
         * @param remove 删除 已经消失的环境 到当前进程
         * @param update 更新 新的变量到当前进程
         */
        extern void updateThis(bool remove = true, bool update = true, std::vector<std::wstring> *filterList = nullptr);

        /**
         * @brief
         *
         * @param key
         * @return true
         * @return false
         */
        extern bool hasSysKeyExists(const std::wstring key);
        /**
         * @brief 判断是否存在此键
         * @param key
         * @return
         */
        extern bool hasUseKeyExists(const std::wstring key);
        /**
         * @brief 判断是否存在此键（不区分系统和用户）
         * ?- 用户变量和系统变量同时有非数组键  -> 用户变量
         * ?- 用户变量和系统变量同时为数组键    -> 用户变量
         * ?- 用户变量为数组键 系统变量为文本键  -> 用户
         * ?- 系统变量为文本键 用户变量为数组    -> 用户变量
         * ?- 系统变量存在 用户变量为空文本      -> 排除
         * @param key 键
         * @return true
         * @return false
         */
        extern bool hasKeyExists(const std::wstring key);

        /**
         * @brief 是个可扩展的值
         * @param key
         * @return
         */
        extern bool hasExpval(const std::wstring key);
    }
}

#endif // MODE_INTERNAL_INCLUDE_HMC_ENV_UTIL_H
