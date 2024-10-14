#include <hmc_path_util.h>
#include <hmc_string_util.h>
#include <hmc_util.h>
#include <regex>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// template
namespace ____O_O____
{
    // using namespace hmc_path_util;

    using T = std::wstring;

    template <typename T>
    static bool isNtPath(const T Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        bool result = false;
        T input = formatSeparator<T>(hmc_string_util::getAsciiPrefix(Input));
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
        T input = formatSeparator<T>(hmc_string_util::getAsciiPrefix(Input));
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
    
    template <typename chVolumeParse__T, typename T>
    // using chVolumeParse__T = hmc_path_util::chVolumeParseW;
    // using T = std::wstring;
    static chVolumeParse__T ParseVolume(const T Input)
    {

        chVolumeParse__T result;
        result.valid = false;

        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        T input2 = formatSeparator<T>(hmc_string_util::getAsciiPrefix(Input));
        T input = hmc_string_util::text_to_lower(input2);

        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        if (input.empty() || input.size() < 8)
            return result;

        // '\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\' -> 'Volume{68de5ae6-0000-0000-0000-100000000000}\\'
        size_t uncSize = hmc_path_util::removeUnc(&input);
        hmc_path_util::removeUnc(&input2);

        if (input.empty() || input.size() < 5)
            return result;

        if (false)
        {
        do_return:
            if (result.valid)
            {
                auto parse = hmc_path_util::parse(result.sub);
                result.base = parse.base;
                result.ext = parse.ext;
                result.name = parse.name;
                result.root = result.resolve;
                result.dir.append(result.root);

                if (result.dir.back() != hmc_path_util::SEPARATOR)
                {
                    result.dir.push_back(hmc_path_util::SEPARATOR);
                }

                result.dir.append(parse.dir);
                result.dir = formatSeparator<T>(result.dir);
            }
            return result;
        }

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

            // ?SUB-> [   \\\\?\\Volume{000000-0000-0000-0000-100000000000}  ]  \\123456789\\\\123456
            if (!result.cuid.empty() && (end_pos + uncSize + 2) < input2.size())
            {
                size_t sub_pos = Input.find(result.cuid, 0);
                if (sub_pos != Input.npos)
                {
                    sub_pos = sub_pos + result.cuid.size() + 1;
                    if (sub_pos > result.cuid.size() && sub_pos != Input.npos && sub_pos < Input.size())
                    {
                        result.sub = T(Input.begin() + sub_pos, Input.end());
                    }
                }
            }

            result.cuid.shrink_to_fit();
            result.valid = true;
            result.resolve = T({hmc_path_util::SEPARATOR, hmc_path_util::SEPARATOR, '?', hmc_path_util::SEPARATOR, 'V', 'o', 'l', 'u', 'm', 'e', '{'});
            result.resolve.append(result.cuid);
            result.resolve.push_back('}');
            result.resolve.push_back(hmc_path_util::SEPARATOR);
        }
        goto do_return;
        return result;
    }

    template <typename chNtParse_T, typename T>
    static chNtParse_T ParseNT(const T Input)
    {
        chNtParse_T result;
        result.valid = false;
        result.index = 0;

        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");

        T input2 = formatSeparator<T>(hmc_string_util::getAsciiPrefix(Input));
        T input = hmc_string_util::text_to_lower(input2);

        // "\\Device\\HarddiskVolume2"
        if (input.empty() || input.size() < 8)
            return result;

        // "\\\\?\\\\Device\\\\HarddiskVolume2" -> '\\Device\\HarddiskVolume2'
        size_t uncSize = hmc_path_util::removeUnc(&input);
        hmc_path_util::removeUnc(&input2);

        if (input.empty() || input.size() < 5)
            return result;

        if (false)
        {
        do_return:
            if (result.valid)
            {
                auto parse = hmc_path_util::parse(result.sub);
                result.base = parse.base;
                result.ext = parse.ext;
                result.name = parse.name;
                result.root = result.resolve;
                result.root.push_back(hmc_path_util::SEPARATOR);
                result.dir.append(result.root);

                if (result.dir.back() != hmc_path_util::SEPARATOR)
                {
                    result.dir.push_back(hmc_path_util::SEPARATOR);
                }

                result.dir.append(parse.dir);
                result.dir = formatSeparator<T>(result.dir);
            }
            return result;
        }

        if (input[0] == '\\')
        {
            // \\ 第一个
            size_t firstSlashPos = input.find('\\', 0);
            if (firstSlashPos != input.npos)
            {

                // \\ 第二个
                size_t secondSlashPos = input.find('\\', firstSlashPos + 1);

                T deviceType = T(input2.begin() + (firstSlashPos + 1), input2.begin() + secondSlashPos);

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

                    T volumeName = T(input.begin() + secondSlashPos + 1, input.end());

                    T HarddiskVolume = T({'h', 'a', 'r', 'd', 'd', 'i', 's', 'k', 'v', 'o', 'l', 'u', 'm', 'e'});
                    size_t volumeNameIndex = volumeName.find(HarddiskVolume);

                    T volumeNameCopy = T(input2.begin() + secondSlashPos + 1, input2.end());
                    if (volumeNameIndex != volumeName.npos)
                    {

                        result.volume_id.reserve(15);
                        std::string index_str;

                        bool is_number_eq = false;
                        size_t root_end_pos = 0;

                        for (size_t i = volumeNameIndex; i < volumeNameCopy.size(); i++)
                        {
                            auto &at = volumeNameCopy.at(i);

                            if (is_number_eq && !(at <= '9' && at >= '0'))
                            {
                                root_end_pos = i;
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

                        // ?SUB-> [   \\\\?\\\\Device\\HarddiskVolume2\\  ]  \\123456789\\\\123456
                        if (!result.volume_id.empty())
                        {
                            size_t sub_pos = Input.find(result.volume_id, 0);
                            if (sub_pos != Input.npos)
                            {
                                sub_pos = sub_pos + result.volume_id.size() + 1;
                                if (sub_pos > result.volume_id.size() && sub_pos != Input.npos && sub_pos < Input.size())
                                {
                                    result.sub = T(Input.begin() + sub_pos, Input.end());
                                }
                            }
                        }

                        result.device_type = std::move(deviceType);
                        // result.resolve = T({'\\','\\','?','\\'});
                        result.resolve.push_back(hmc_path_util::SEPARATOR);
                        result.resolve.append(result.device_type);
                        result.resolve.push_back(hmc_path_util::SEPARATOR);
                        result.resolve.append(result.volume_id);
                        result.resolve_unc.append(T({hmc_path_util::SEPARATOR, hmc_path_util::SEPARATOR, '?', hmc_path_util::SEPARATOR}));
                        result.resolve_unc.append(result.resolve);
                        result.valid = true;
                    }
                }
            }
        }

        goto do_return;
        return result;
    }

    template <typename T>
    static T baseName(const T _path)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();
        T path = formatSeparator<T>(_path);
        long long length = path.size();

        if (path.empty())
        {
            return result;
        }

        for (long long _i = length - 1; _i >= 0; _i--)
        {
            size_t i = static_cast<size_t>(_i);

            // 防止有可能的 -1 越界
            if (i >= path.size())
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
    static T extName(const T _path)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();

        T path = formatSeparator<T>(_path);
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
        T input = formatSeparator<T>(T(Input.begin(), Input.end()));

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

    template <typename chParse__T, typename T>
    static chParse__T Parse(const T _Input)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        chParse__T result = chParse__T();

        if (_Input.empty())
        {
            return result;
        }

        T Input = formatSeparator<T>(_Input);

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

        T Input = formatSeparator<T>(_Input);

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
        T input = formatSeparator<T>(hmc_string_util::getAsciiPrefix(Input));

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

    template <typename T>
    static T formatSeparator(const T _Input)
    {
        T Input;
        Input.reserve(_Input.size());

        for (size_t i = 0; i < _Input.size(); ++i)
        {

            if (i >= _Input.size())
                break;

            auto &at = _Input.at(i);

            if (at == '\0')
            {
                continue;
            }

            if (at == '\\' || at == '/')
            {
                auto sep = 0 + hmc_path_util::SEPARATOR;
                Input.push_back(sep);
                continue;
            }

            Input.push_back(at);
        }

        T result;
        if (Input.empty())
        {
            return result;
        }

        result.reserve(Input.size());

        size_t start = 0;

        // \\?\

        if constexpr (std::is_same_v<T, std::wstring>)
        {
            if (start == 0 && Input.compare(0, 5, L"\\\\?\\\\") == 0)
            {
                result.append(Input, 0, 5);
                start = 5;
            }

            if (start == 0 && Input.compare(0, 4, L"\\\\?\\") == 0)
            {
                result.append(Input, 0, 4);
                start = 4;
            }
        }

        if constexpr (std::is_same_v<T, std::string>)
        {
            if (start == 0 && Input.compare(0, 5, "\\\\?\\\\") == 0)
            {
                result.append(Input, 0, 5);
                start = 5;
            }

            if (start == 0 && Input.compare(0, 4, "\\\\?\\") == 0)
            {
                result.append(Input, 0, 4);
                start = 4;
            }
        }

        for (size_t i = start; i < Input.size(); ++i)
        {

            if (i >= Input.size())
            {
                break;
            }

            auto it = Input.at(i);

            if (i > 0 && it == hmc_path_util::SEPARATOR && result.back() == hmc_path_util::SEPARATOR)
            {
                continue;
            }

            result.push_back(it);
        }

        if constexpr (std::is_same_v<T, std::wstring>)
        {
            if (result.compare(0, 3, L"\\?\\") == 0)
            {
                result.insert(0, 1, '\\');
            }
        }

        if constexpr (std::is_same_v<T, std::string>)
        {
            if (result.compare(0, 3, "\\?\\") == 0)
            {
                result.insert(0, 1, '\\');
            }
        }

        return result;
    }

    template <typename T>
    static size_t removeUnc(T *Input)
    {
        size_t result = 0;
        if (Input == NULL || Input->size() < 5)
            return result;

        if (hmc_path_util::isUncPath(*Input))
        {
            Input->erase(0, 4);
            result = 4;
            return result;
        }
        return result;
    }

    template <typename T>
    static bool isDosPath(T Input)
    {
        bool result = false;
        T input = hmc_string_util::getAsciiPrefix(Input);

        bool IS_UNC_PRE_FIX = hmc_path_util::isUncPath(input);

        if (IS_UNC_PRE_FIX)
        {
            if (input.size() > 5)
            {
                if constexpr (std::is_same_v<T, std::string>)
                {
                    const char AT = input[4];
                    const char FIX = input[5];

                    if (FIX == ':' && hmc_string_util::is_letter(AT))
                    {
                        result = true;
                        return result;
                    }
                }
                if constexpr (std::is_same_v<T, std::wstring>)
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
        }
        if constexpr (std::is_same_v<T, std::wstring>)
        {
            const wchar_t AT = input[0];
            const wchar_t FIX = input[1];

            if (FIX == ':' && hmc_string_util::is_letter(AT))
            {
                result = true;
            }
        }
        if constexpr (std::is_same_v<T, std::string>)
        {
            const char AT = input[0];
            const char FIX = input[1];

            if (FIX == ':' && hmc_string_util::is_letter(AT))
            {
                result = true;
            }
        }

        return result;
    }

    template <typename T>
    static bool isUncPath(T Input)
    {
        bool result = false;
        T input = hmc_string_util::getAsciiPrefix(Input);
        if (input.size() > 4)
        {
            result = input[0] == L'\\' && input[1] == L'\\' && input[2] == L'?' && input[3] == L'\\';
        }
        return result;
    }

}

bool hmc_path_util::isUncPath(const std::string Input)
{
    return ____O_O____::isUncPath<std::string>(Input);
}

bool hmc_path_util::isUncPath(const std::wstring Input)
{
    return ____O_O____::isUncPath<std::wstring>(Input);
}

bool hmc_path_util::isDosPath(const std::wstring Input)
{
    return ____O_O____::isDosPath<std::wstring>(Input);
}

bool hmc_path_util::isDosPath(const std::string Input)
{
    return ____O_O____::isDosPath<std::string>(Input);
}

size_t hmc_path_util::removeUnc(std::string *Input)
{
    return ____O_O____::removeUnc<std::string>(Input);
}

size_t hmc_path_util::removeUnc(std::wstring *Input)
{
    return ____O_O____::removeUnc<std::wstring>(Input);
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

hmc_path_util::chVolumeParseW hmc_path_util::parseVolume(const std::wstring Input)
{
    return ____O_O____::ParseVolume<hmc_path_util::chVolumeParseW, std::wstring>(Input);
}

hmc_path_util::chVolumeParseA hmc_path_util::parseVolume(const std::string Input)
{
    return ____O_O____::ParseVolume<hmc_path_util::chVolumeParseA, std::string>(Input);
}

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

std::string hmc_path_util::formatSeparator(const std::string Input)
{
    return ____O_O____::formatSeparator<std::string>(Input);
}

std::wstring hmc_path_util::formatSeparator(const std::wstring Input)
{
    return ____O_O____::formatSeparator<std::wstring>(Input);
}

void hmc_path_util::formatSeparator(std::string *Input)
{
    auto result = ____O_O____::formatSeparator<std::string>(*Input);
    *Input = result;
}

void hmc_path_util::formatSeparator(std::wstring *Input)
{
    auto result = ____O_O____::formatSeparator<std::wstring>(*Input);
    *Input = result;
}
