#ifndef HMC_IMPORT_HMC_PATH_UTIL_H
#define HMC_IMPORT_HMC_PATH_UTIL_H
#include <string>
#include <vector>
#include <map>
#include <Windows.h>

namespace hmc_path_util
{


#if defined(_WIN32) || defined(__CYGWIN__)
    static const bool IS_WIN32 = true;
    static const char SEPARATOR = '\\';
    static const char DELIMITER = ':';
#else
    static const bool IS_WIN32 = false;
    static const char SEPARATOR = '/';
    static const char DELIMITER = ':';
#endif

    struct chParseW
    {
        std::wstring base;
        std::wstring dir;
        std::wstring ext;
        std::wstring name;
        std::wstring root;
    };

    struct chParseA
    {
        std::string base;
        std::string dir;
        std::string ext;
        std::string name;
        std::string root;
    };

    struct chVolumeParseA : public chParseA
    {
        std::string cuid;
        std::string resolve;
        std::string sub;
        bool valid;
    };

    struct chVolumeParseW : public chParseW
    {
        std::wstring cuid;
        std::wstring resolve;
        std::wstring sub;
        bool valid;
    };

    struct chNtParseA  : public chParseA
    {
        std::string volume_id;
        std::string device_type;
        std::string resolve;
        std::string resolve_unc;
        std::string sub;
        std::uint32_t index;
        bool valid;
        chNtParseA():valid(false), index(0){}
    };

    struct chNtParseW : public chParseW
    {
        std::wstring volume_id;
        std::wstring device_type;
        std::wstring resolve;
        std::wstring resolve_unc;
        std::wstring sub;
        std::uint32_t index;
        bool valid;
        chNtParseW() :valid(false), index(0) {}
    };

    /**
     * @brief 判断是否是 \\\\?\\ 开头 并忽略空白字符
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isUncPath(const std::wstring Input);
    /**
     * @brief 判断是否是 \\\\?\\ 开头 并忽略空白字符
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isUncPath(const std::string Input);
    /**
     * @brief 删除开头的  \\\\?\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern size_t removeUnc(std::wstring *Input);
    /**
     * @brief 删除开头的  \\\\?\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern size_t removeUnc(std::string *Input);

    /**
     * @brief 替换为win标准的 \\ 路径分隔符号
     * @example \\\\/ -> \\
     * @example \\\\?\\\\123 -> \\\\?\\123
     * @example /1234\\2344  -> \\1234\\2344
     * @example C:\\/\\Users\\Example -> C:\\Users\\Example
     * 
     * @param Input 
     * @return size_t 
     */
    extern void formatSeparator(std::wstring *Input);
    /**
     * @brief 替换为win标准的 \\ 路径分隔符号
     * @example \\\\/ -> \\
     * @example \\\\?\\\\123 -> \\\\?\\123
     * @example /1234\\2344  -> \\1234\\2344
     * @example C:\\/\\Users\\Example -> C:\\Users\\Example
     * 
     * @param Input 
     * @return size_t 
     */
    extern void formatSeparator(std::string *Input);
    /**
     * @brief 替换为win标准的 \\ 路径分隔符号
     * @example \\\\/ -> \\
     * @example \\\\?\\\\123 -> \\\\?\\123
     * @example /1234\\2344  -> \\1234\\2344
     * @example C:\\/\\Users\\Example -> C:\\Users\\Example
     * 
     * @param Input 
     * @return size_t 
     */
    extern std::wstring formatSeparator(const std::wstring Input);
    /**
     * @brief 替换为win标准的 \\ 路径分隔符号
     * @example \\\\/ -> \\
     * @example \\\\?\\\\123 -> \\\\?\\123
     * @example /1234\\2344  -> \\1234\\2344
     * @example C:\\/\\Users\\Example -> C:\\Users\\Example
     * 
     * @param Input 
     * @return size_t 
     */
    extern std::string formatSeparator(const std::string Input);

    /**
     * @brief 判断是否是 \\\\?\\ 开头的标准dos路径 并忽略空白字符
     * @example \\\\?\\F:\\sss\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isDosPath(const std::wstring Input);
    /**
     * @brief 判断是否是 \\\\?\\ 开头的标准dos路径 并忽略空白字符
     * @example \\\\?\\F:\\sss\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isDosPath(const std::string Input);
    /**
     * @brief 判断是否是 Nt 开头 并忽略空白字符
     * @example \\?\\\\Device\\HarddiskVolume2
     * @example \\Device\\HarddiskVolume2
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isNtPath(const std::wstring Input);
    /**
     * @brief 判断是否是 Nt 开头 并忽略空白字符
     * @example \\?\\\\Device\\HarddiskVolume2
     * @example \\Device\\HarddiskVolume2
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isNtPath(const std::string Input);
    /**
     * @brief 判断是否是 Volume 开头 并忽略空白字符
     * @example \\?\\Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\\
     * @example Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isVolumePath(const std::wstring Input);
    /**
     * @brief 判断是否是 Volume 开头 并忽略空白字符
     * @example \\?\\Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\\
     * @example Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\\
     *
     * @param Input
     * @return true
     * @return false
     */
    extern bool isVolumePath(const std::string Input);
    /**
     * @brief 获取根路径 例如 D:\\ , \\Device\\HarddiskVolume2\\ ,'\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\'
     *
     * @param Input
     * @return std::wstring
     */
    extern std::wstring getRootPath(const std::wstring Input);
    /**
     * @brief 获取根路径 例如 D:\\ , \\Device\\HarddiskVolume2\\ ,'\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\'
     *
     * @param Input
     * @return std::wstring
     */
    extern std::string getRootPath(const std::string Input);
    /**
     * @brief 判断是否是路径分隔符号
     *
     * @param s
     * @return true
     * @return false
     */
    extern bool isPathSeparator(const wchar_t s);

    /**
     * @brief 判断是否是路径分隔符号
     *
     * @param s
     * @return true
     * @return false
     */
    extern bool isPathSeparator(const char s);

    /**
     * @brief 解析 Volume 路径
     * 
     * @param Input 
     * @return chVolumeParseA 
     */
    extern chVolumeParseA parseVolume(const std::string Input);
    /**
     * @brief 解析 Volume 路径
     * 
     * @param Input 
     * @return chVolumeParseA 
     */
    extern chVolumeParseW parseVolume(const std::wstring Input);
    /**
     * @brief 解析 Nt驱动器 路径
     * 
     * @param Input 
     * @return chNtParseA 
     */
    extern chNtParseA parseNt(const std::string Input);
    /**
     * @brief 解析 Nt驱动器 路径
     * 
     * @param Input 
     * @return chNtParseA 
     */
    extern chNtParseW parseNt(const std::wstring Input);

    /**
     * @brief 解析路径
     * 
     * @param Input 
     * @return chNtParseA 
     */
    extern chParseA parse(const std::string Input);
    /**
     * @brief 解析路径
     * 
     * @param Input 
     * @return chNtParseA 
     */
    extern chParseW parse(const std::wstring Input);
    /**
     * @brief 分割路径为数组
     *
     * @param path
     * @return std::string
     */
    extern std::vector<std::wstring> splitPath(const std::wstring path);
    /**
     * @brief 合并路径 并且不对其进行其他处理
     *
     * @param list
     * @return String
     */
    extern std::wstring joinPathList(const std::vector<std::wstring> &list);
    /**
     * @brief 分割路径为数组
     *
     * @param path
     * @return std::string
     */
    extern std::vector<std::string> splitPath(const std::string path);
    /**
     * @brief 合并路径 并且不对其进行其他处理
     *
     * @param list
     * @return String
     */
    extern std::string joinPathList(const std::vector<std::string> &list);
    /**
     * @brief 获取文件名 全名
     *
     * @param path
     * @return String
     */
    extern std::string baseName(const std::string path);
    /**
     * @brief 获取文件名 全名
     *
     * @param path
     * @return String
     */
    extern std::wstring baseName(const std::wstring path);
    /**
     * @brief 提供特定于平台的路径定界符：
     *
     * @return String
     */
    extern char delimiter();
    /**
     * @brief 方法返回 path 的扩展名
     * @details 即 path 的最后一部分中从最后一次出现的 .（句点）字符到字符串的结尾。如果 path 的最后一部分中没有 .，或者除了 path 的基本名称（参见 path.basename()）的第一个字符之外没有 . 个字符，则返回空字符串
     * @param path
     * @return String
     */
    extern std::string extName(const std::string path);
    /**
     * @brief 方法返回 path 的扩展名
     * @details 即 path 的最后一部分中从最后一次出现的 .（句点）字符到字符串的结尾。如果 path 的最后一部分中没有 .，或者除了 path 的基本名称（参见 path.basename()）的第一个字符之外没有 . 个字符，则返回空字符串
     * @param path
     * @return String
     */
    extern std::wstring extName(const std::wstring path);
    /**
     * @brief 方法返回 path 的目录名，类似于 Unix dirname 命令。尾随的目录分隔符被忽略，见 path.sep
     *
     * @param path
     * @return String
     */
    extern std::wstring dirName(const std::wstring path);
    /**
     * @brief 方法返回 path 的目录名，类似于 Unix dirname 命令。尾随的目录分隔符被忽略，见 path.sep
     *
     * @param path
     * @return String
     */
    extern std::string dirName(const std::string path);
    /**
     * @brief 方法返回 path 的目录名，类似于 Unix dirname 命令。尾随的目录分隔符被忽略，见 path.sep
     *
     * @param path
     * @return String
     */
    extern std::wstring rootName(const std::wstring path);
    /**
     * @brief 方法返回 path 的目录名，类似于 Unix dirname 命令。尾随的目录分隔符被忽略，见 path.sep
     *
     * @param path
     * @return String
     */
    extern std::string rootName(const std::string path);
    /**
     * @brief 当前平台的路径分割符号
     * 
     * @return char 
     */
    extern char sep();
}

#endif // HMC_IMPORT_HMC_PATH_UTIL_H
