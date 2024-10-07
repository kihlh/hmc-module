#include <hmc_path_util.h>
#include <hmc_string_util.h>
#include <regex>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// template
namespace ____O_O____
{
    // using namespace hmc_path_util;

    using T = std::wstring;

    static std::wstring GetAllBeginAscii(const std::wstring input);
    static std::string GetAllBeginAscii(const std::string input);

    template <typename T>
    static T HMC_GET_ALL_BEGIN_ASCII(const T input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        T result;
        if (input.empty())
            return result;
        result.reserve(input.size());
        for (size_t i = 0; i < input.size(); i++)
        {
            auto it = input.at(i);

            if (it == '\0')
            {
                continue;
            }
            if (!hmc_string_util::is_ascii(it))
            {
                break;
            }
            result.push_back(it);
        }

        result.shrink_to_fit();

        return result;
    }

    template <typename T>
    static bool isNtPath(const T Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        bool result = false;
        T input = GetAllBeginAscii(Input);
        input = hmc_string_util::text_to_lower(input);

        // "\\Device\\HarddiskVolume2"
        if (input.empty() || input.size() < 8)
            return result;

        // "\\\\?\\\\Device\\\\HarddiskVolume2" -> '\\Device\\HarddiskVolume2'
        hmc_path_util::removeUnc(&input);

        if (input.empty() || input.size() < 5)
            return result;

        if (input[0] == hmc_path_util::SEPARATOR)
        {
            // \\ 第一个
            size_t firstSlashPos = input.find(hmc_path_util::SEPARATOR, 0);
            if (firstSlashPos != input.npos)
            {
                // \\ 第二个
                size_t secondSlashPos = input.find(hmc_path_util::SEPARATOR, firstSlashPos + 1);

                if (secondSlashPos != input.npos)
                {

                    if ((secondSlashPos + 2) >= input.size())
                    {
                        return result;
                    }

                    T volumeName = T(input.begin() + secondSlashPos + 1, input.end());

                    T HarddiskVolume = T({'h', 'a', 'r', 'd', 'd', 'i', 's', 'k', 'v', 'o', 'l', 'u', 'm', 'e'});
                    if (volumeName.find(HarddiskVolume) != volumeName.npos)
                    {
                        result = true;
                    }
                }
            }
        }

        return result;
    }

    template <typename T>
    static bool isVolumePath(const T Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        bool result = false;
        T input = GetAllBeginAscii(Input);
        input = hmc_string_util::text_to_lower(input);
        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        if (input.empty() || input.size() < 8)
            return result;

        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\' -> 'Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        hmc_path_util::removeUnc(&input);

        if (input.empty() || input.size() < 5)
            return result;

        const T voiumeName = T({'v', 'o', 'l', 'u', 'm', 'e'});
        // \\ 第一个
        size_t firstSlashPos = input.find(voiumeName, 0);
        if (firstSlashPos != input.npos)
        {
            // \\ 第二个
            size_t secondSlashPos = firstSlashPos + voiumeName.size();

            if (secondSlashPos >= input.size())
            {
                return result;
            }

            secondSlashPos = input.find('{', secondSlashPos);

            if (secondSlashPos >= input.size())
            {
                return result;
            }

            if (input[secondSlashPos] != '{')
            {
                return result;
            }

            for (size_t i = secondSlashPos + 1; i < input.size(); i++)
            {
                auto &at = input[i];

                if (at == '}')
                {
                    result = true;
                    break;
                }
            }
        }

        return result;
    }

    template <typename chVolumeParse__T, typename ThisString__T>
    static chVolumeParse__T ParseVolume(const ThisString__T Input)
    {

        chVolumeParse__T result = {ThisString__T(), ThisString__T(), false};

        static_assert(std::is_same_v<ThisString__T, std::string> || std::is_same_v<ThisString__T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        ThisString__T input2 = GetAllBeginAscii(Input);
        ThisString__T input = hmc_string_util::text_to_lower(input2);

        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        if (input.empty() || input.size() < 8)
            return result;

        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\' -> 'Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        hmc_path_util::removeUnc(&input);
        hmc_path_util::removeUnc(&input2);

        if (input.empty() || input.size() < 5)
            return result;

        const ThisString__T voiumeName = ThisString__T({'v', 'o', 'l', 'u', 'm', 'e'});

        // \\ 第一个
        size_t firstSlashPos = input.find(voiumeName, 0);
        if (firstSlashPos != input.npos)
        {
            // \\ 第二个
            size_t secondSlashPos = firstSlashPos + voiumeName.size();

            if (secondSlashPos >= input.size())
            {
                return result;
            }

            secondSlashPos = input.find('{', secondSlashPos);

            if (secondSlashPos >= input.size())
            {
                return result;
            }

            if (input[secondSlashPos] != '{')
            {
                return result;
            }

            size_t end_pos = secondSlashPos + 1;

            for (; end_pos < input.size(); end_pos++)
            {
                auto &at = input[end_pos];

                if (at == '}')
                {
                    break;
                }
            }

            result.cuid.reserve(input.size());

            for (size_t i = secondSlashPos + 1; i < end_pos; i++)
            {
                auto &at = input2.at(i);
                result.cuid.push_back(at);
            }
            result.cuid.shrink_to_fit();
            result.valid = true;
            result.resolve = ThisString__T({hmc_path_util::SEPARATOR, hmc_path_util::SEPARATOR, '?', hmc_path_util::SEPARATOR, 'V', 'o', 'l', 'u', 'm', 'e', '{'});
            result.resolve.append(result.cuid);
            result.resolve.push_back('}');
            result.resolve.push_back(hmc_path_util::SEPARATOR);
        }

        return result;
    }

    template <typename chNtParse_T, typename ThisString_T>
    static chNtParse_T ParseNT(const ThisString_T Input)
    {
        chNtParse_T result = {ThisString_T(), ThisString_T(), ThisString_T(), ThisString_T(), 0, false};
        static_assert(std::is_same_v<ThisString_T, std::string> || std::is_same_v<ThisString_T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        ThisString_T input2 = GetAllBeginAscii(Input);
        ThisString_T input = hmc_string_util::text_to_lower(input2);

        // "\\Device\\HarddiskVolume2"
        if (input.empty() || input.size() < 8)
            return result;

        // "\\\\?\\\\Device\\\\HarddiskVolume2" -> '\\Device\\HarddiskVolume2'
        hmc_path_util::removeUnc(&input);
        hmc_path_util::removeUnc(&input2);

        if (input.empty() || input.size() < 5)
            return result;

        if (input[0] == '\\')
        {
            // \\ 第一个
            size_t firstSlashPos = input.find('\\', 0);
            if (firstSlashPos != input.npos)
            {

                // \\ 第二个
                size_t secondSlashPos = input.find('\\', firstSlashPos + 1);

                ThisString_T deviceType = ThisString_T(input2.begin() + (firstSlashPos + 1), input2.begin() + secondSlashPos);

                if (deviceType.empty())
                {
                    return result;
                }

                if (secondSlashPos != input.npos)
                {

                    if ((secondSlashPos + 2) >= input.size())
                    {
                        return result;
                    }

                    ThisString_T volumeName = ThisString_T(input.begin() + secondSlashPos + 1, input.end());

                    ThisString_T HarddiskVolume = ThisString_T({'h', 'a', 'r', 'd', 'd', 'i', 's', 'k', 'v', 'o', 'l', 'u', 'm', 'e'});
                    size_t volumeNameIndex = volumeName.find(HarddiskVolume);

                    ThisString_T volumeNameCopy = ThisString_T(input2.begin() + secondSlashPos + 1, input2.end());
                    if (volumeNameIndex != volumeName.npos)
                    {

                        result.volume_id.reserve(15);
                        std::string index_str;

                        bool is_number_eq = false;
                        for (size_t i = volumeNameIndex; i < volumeNameCopy.size(); i++)
                        {
                            auto &at = volumeNameCopy.at(i);

                            if (is_number_eq && hmc_string_util::is_letter(at))
                            {
                                break;
                            }

                            result.volume_id.push_back(at);

                            if (at <= '9' && at >= '0')
                            {
                                index_str.push_back(static_cast<char>(at));
                                is_number_eq = true;
                            }
                        }

                        result.index = std::stoi(index_str);

                        if (deviceType.empty() || result.volume_id.empty())
                        {
                            return result;
                        }

                        result.device_type = std::move(deviceType);
                        // result.resolve = ThisString_T({'\\','\\','?','\\'});
                        result.resolve.push_back(hmc_path_util::SEPARATOR);
                        result.resolve.append(result.device_type);
                        result.resolve.push_back(hmc_path_util::SEPARATOR);
                        result.resolve.append(result.volume_id);
                        result.resolve_unc.append(ThisString_T({hmc_path_util::SEPARATOR, hmc_path_util::SEPARATOR, '?', hmc_path_util::SEPARATOR}));
                        result.resolve_unc.append(result.resolve);
                        result.valid = true;
                    }
                }
            }
        }

        return result;
    }

    /**
     * @brief 提取开头的所有ASCII字符串 并忽略空白字符 直到超过 127
     *
     * @param input
     * @return std::wstring
     */
    static std::wstring GetAllBeginAscii(const std::wstring input)
    {
        return HMC_GET_ALL_BEGIN_ASCII<std::wstring>(input);
    }

    /**
     * @brief 提取开头的所有ASCII字符串 并忽略空白字符 直到超过 127
     *
     * @param input
     * @return std::string
     */
    static std::string GetAllBeginAscii(const std::string input)
    {
        return HMC_GET_ALL_BEGIN_ASCII<std::string>(input);
    }

    template <typename T>
    static T baseName(const T path)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();

        long long length = path.size();

        if (path.empty())
        {
            return result;
        }

        for (long long _i = length - 1; _i >= 0; _i--)
        {
            size_t i = static_cast<size_t>(_i);

            // 防止有可能的 -1 越界
            if (i > path.size())
            {
                break;
            }


            if constexpr (std::is_same_v<T, std::wstring>)
            {
                wchar_t it = static_cast<wchar_t>(path.at(i));

                if (it == '\0')
                {
                    continue;
                }

                if (hmc_path_util::isPathSeparator(it))
                {
                    break;
                }

                result.insert(0, 1, it);
            }

            if constexpr (std::is_same_v<T, std::string>)
            {
                char it = path.at(i);

                if (it == '\0')
                {
                    continue;
                }

                if (hmc_path_util::isPathSeparator(it))
                {
                    break;
                }

                result.insert(0, 1, it);
            }
        }

        return result;
    }

    template <typename T>
    static T extName(const T path)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();

        if (path.empty() || hmc_path_util::isPathSeparator(path.back()))
        {
            return result;
        }

        auto temp = hmc_path_util::baseName(path);

        if (temp.empty())
        {
            return result;
        }

        size_t dotIdx = temp.find_last_of(T(1, '.'));

        if (dotIdx != temp.npos)
        {

            if (dotIdx >= temp.size())
            {
                return result;
            }

            result.assign(temp.begin() + dotIdx, temp.end());
        }

        return result;
    }

    template <typename T>
    static std::vector<T> splitPath(const T path)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        std::vector<T> result;

        size_t count = path.size();

        T temp = T();

        for (size_t i = 0; i < count; i++)
        {
            auto &&it = path.at(i);

            if (hmc_path_util::isPathSeparator(it) || it == '\0')
            {
                if (!temp.empty())
                {
                    result.emplace_back(std::move(temp));
                    temp.clear();
                }
            }
            else
            {
                temp.push_back(it);
            }
        }
        if (!temp.empty())
        {
            result.emplace_back(std::move(temp));
        }
        return result;
    }

    template <typename T>
    static T joinPathList(const std::vector<T> pathList)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result;

        size_t max_size = hmc_string_util::size(pathList);
        result.reserve(max_size);

        for (auto &it : pathList)
        {
            result.append(it);
            result.push_back(hmc_path_util::SEPARATOR);
        }

        if (pathList.size() > 0)
        {
            result.pop_back();
        }

        return result;
    }

    template <typename T>
    static T rootName(const T Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();
        T input = T(Input.begin(), Input.end());

        if (input.empty())
        {
            return result;
        }

        if (hmc_path_util::isUncPath(input))
        {
            hmc_path_util::removeUnc(&input);
        }

        for (size_t i = 0; i < input.size(); i++)
        {
            auto &at = input.at(i);
            result.push_back(at);
            if (at == hmc_path_util::DELIMITER)
            {
                break;
            }
        }

        if (result.empty())
        {
            return result;
        }

        result.push_back(hmc_path_util::SEPARATOR);

        return result;
    }

    template <typename chParse__T, typename ThisString__T>
    static chParse__T Parse(const ThisString__T Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        chParse__T result = chParse__T();

        if (Input.empty())
        {
            return result;
        }

        result.root = hmc_path_util::rootName(Input);
        result.dir = hmc_path_util::dirName(Input);
        result.base = hmc_path_util::baseName(Input);
        result.ext = hmc_path_util::extName(Input);
        result.name = result.base.substr(0, result.base.length() - result.ext.length());
        return result;
    }

    template <typename T>
    static T dirName(const T _Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        T result = T();

        T Input = T(_Input.begin(), _Input.end());

        if (hmc_path_util::isUncPath(Input))
        {
            hmc_path_util::removeUnc(&Input);
        }

        if (Input.empty())
        {
            return result;
        }

        if constexpr (std::is_same_v<T, std::wstring>)
        {
            wchar_t path[MAX_PATH];

            ::wcsncpy_s(path, Input.c_str(), MAX_PATH);

            if (::PathRemoveFileSpecW(path))
            {
                result = T(path);
            }
            else
            {
                return result;
            }
        }

        if constexpr (std::is_same_v<T, std::string>)
        {
            char path[MAX_PATH];
            ::strncpy_s(path, Input.c_str(), MAX_PATH);
            if (::PathRemoveFileSpecA(path))
            {
                result = T(path);
            }
            else
            {
                return result;
            }
        }

        return result;
    }

    template <typename T>
    static T getRootPath(const T Input)
    {
        T input = GetAllBeginAscii(Input);

        T result;
        if (hmc_path_util::isVolumePath(input))
        {
            auto parse = hmc_path_util::parseVolume(input);
            if (parse.valid)
            {
                result.append(parse.resolve);
                return result;
            }
            return result;
        }

        if (hmc_path_util::isNtPath(input))
        {
            auto parse = hmc_path_util::parseNt(input);
            if (parse.valid)
            {
                result.append(parse.resolve);
            }

            return result;
        }

        auto parse = hmc_path_util::parse(input);

        if (!parse.root.empty())
        {
            result.append(parse.root);
            return result;
        }

        return result;
    }

}

bool hmc_path_util::isUncPath(const std::string Input)
{
    bool result = false;
    std::string input = ____O_O____::GetAllBeginAscii(Input);

    if (input.size() > 4)
    {
        result = input[0] == L'\\' && input[1] == L'\\' && input[2] == L'?' && input[3] == L'\\';
    }
    return result;
}

bool hmc_path_util::isUncPath(const std::wstring Input)
{
    bool result = false;
    std::wstring input = ____O_O____::GetAllBeginAscii(Input);
    if (input.size() > 4)
    {
        result = input[0] == L'\\' && input[1] == L'\\' && input[2] == L'?' && input[3] == L'\\';
    }
    return result;
}

bool hmc_path_util::isDosPath(const std::wstring Input)
{
    bool result = false;
    std::wstring input = ____O_O____::GetAllBeginAscii(Input);

    bool IS_UNC_PRE_FIX = isUncPath(input);

    if (IS_UNC_PRE_FIX)
    {
        if (input.size() > 5)
        {
            const wchar_t AT = input[4];
            const wchar_t FIX = input[5];

            if (FIX == ':' && hmc_string_util::is_letter(AT))
            {
                result = true;
                return result;
            }
        }
    }

    const wchar_t AT = input[0];
    const wchar_t FIX = input[1];

    if (FIX == ':' && hmc_string_util::is_letter(AT))
    {
        result = true;
    }

    return result;
}

bool hmc_path_util::isDosPath(const std::string Input)
{
    bool result = false;
    std::string input = ____O_O____::GetAllBeginAscii(Input);

    bool IS_UNC_PRE_FIX = isUncPath(input);

    if (IS_UNC_PRE_FIX)
    {
        if (input.size() > 5)
        {
            const char AT = input[4];
            const char FIX = input[5];

            if (FIX == ':' && ((AT >= L'A' && AT <= L'Z') || (AT >= L'a' && AT <= L'z')))
            {
                result = true;
                return result;
            }
        }
    }

    const char AT = input[0];
    const char FIX = input[1];

    if (FIX == ':' && hmc_string_util::is_letter(AT))
    {
        result = true;
    }

    return result;
}

void hmc_path_util::removeUnc(std::string *Input)
{
    if (Input == NULL || Input->size() < 5)
        return;

    if (isUncPath(*Input))
    {
        Input->erase(0, 4);
    }
}

void hmc_path_util::removeUnc(std::wstring *Input)
{
    if (Input == NULL || Input->size() < 5)
        return;

    if (isUncPath(*Input))
    {
        Input->erase(0, 4);
    }
}

bool hmc_path_util::isNtPath(const std::wstring Input)
{
    return ____O_O____::isNtPath<std::wstring>(Input);
}

bool hmc_path_util::isNtPath(const std::string Input)
{
    return ____O_O____::isNtPath<std::string>(Input);
}

bool hmc_path_util::isVolumePath(const std::wstring Input)
{
    return ____O_O____::isVolumePath<std::wstring>(Input);
}

bool hmc_path_util::isVolumePath(const std::string Input)
{
    return ____O_O____::isVolumePath<std::string>(Input);
}

hmc_path_util::chVolumeParseA hmc_path_util::parseVolume(const std::string Input);

hmc_path_util::chVolumeParseW hmc_path_util::parseVolume(const std::wstring Input)
{
    return ____O_O____::ParseVolume<hmc_path_util::chVolumeParseW, std::wstring>(Input);
}

hmc_path_util::chVolumeParseA hmc_path_util::parseVolume(const std::string Input)
{
    return ____O_O____::ParseVolume<hmc_path_util::chVolumeParseA, std::string>(Input);
}

// using ThisString_T = std::wstring;
// using chNtParse_T = hmc_path_util::chNtParseW;

hmc_path_util::chNtParseA hmc_path_util::parseNt(const std::string Input)
{
    return ____O_O____::ParseNT<hmc_path_util::chNtParseA, std::string>(Input);
}

hmc_path_util::chNtParseW hmc_path_util::parseNt(const std::wstring Input)
{
    return ____O_O____::ParseNT<hmc_path_util::chNtParseW, std::wstring>(Input);
}

char hmc_path_util::sep()
{
    return SEPARATOR + 0;
}

hmc_path_util::chParseA hmc_path_util::parse(const std::string Input)
{
    return ____O_O____::Parse<hmc_path_util::chParseA, std::string>(Input);
}

hmc_path_util::chParseW hmc_path_util::parse(const std::wstring Input)
{
    return ____O_O____::Parse<hmc_path_util::chParseW, std::wstring>(Input);
}

std::wstring hmc_path_util::baseName(const std::wstring path)
{
    return ____O_O____::baseName<std::wstring>(path);
}

std::string hmc_path_util::baseName(const std::string path)
{
    return ____O_O____::baseName<std::string>(path);
}

inline bool hmc_path_util::isPathSeparator(const char s)
{
    return s == '/' || s == '\\';
}

inline bool hmc_path_util::isPathSeparator(const wchar_t s)
{
    return s == '/' || s == '\\';
}

std::wstring hmc_path_util::extName(const std::wstring path)
{
    return ____O_O____::extName<std::wstring>(path);
}

std::string hmc_path_util::extName(const std::string path)
{
    return ____O_O____::extName<std::string>(path);
}

std::vector<std::wstring> hmc_path_util::splitPath(const std::wstring path)
{
    return ____O_O____::splitPath<std::wstring>(path);
}

std::vector<std::string> hmc_path_util::splitPath(const std::string path)
{
    return ____O_O____::splitPath<std::string>(path);
}

std::string hmc_path_util::joinPathList(const std::vector<std::string> &list)
{
    return ____O_O____::joinPathList<std::string>(list);
}

std::wstring hmc_path_util::joinPathList(const std::vector<std::wstring> &list)
{
    return ____O_O____::joinPathList<std::wstring>(list);
}

std::string hmc_path_util::rootName(const std::string path)
{
    return ____O_O____::rootName<std::string>(path);
}

std::wstring hmc_path_util::rootName(const std::wstring path)
{
    return ____O_O____::rootName<std::wstring>(path);
}

std::string hmc_path_util::dirName(const std::string Input)
{
    return ____O_O____::dirName<std::string>(Input);
}

std::wstring hmc_path_util::dirName(const std::wstring Input)
{
    return ____O_O____::dirName<std::wstring>(Input);
}

std::wstring hmc_path_util::getRootPath(const std::wstring Input)
{
    return ____O_O____::getRootPath<std::wstring>(Input);
}

std::string hmc_path_util::getRootPath(const std::string Input)
{
    return ____O_O____::getRootPath<std::string>(Input);
}
