#include "hmc_json_util.h"
#include "hmc_string_util.h"
#include <windows.h>
#include <crtdbg.h> // For _ASSERTE()
#include <iomanip>
#include <limits>    // For numeric_limits
#include <stdexcept> // For overflow_error
#include <winternl.h>
#include <regex>

namespace hmc_string_util
{

    namespace hmc_define_util
    {
        // 自动释放dll
#define hmc_shared_close_Library(hModule) std::shared_ptr<void>##hModule##_shared_close_Library_(nullptr, [&](void *) {if (hModule != NULL) {::FreeLibrary(hModule);} });
// 自动释放文本
#define hmc_shared_close_lpsz(lpwsz) std::shared_ptr<void>##lpwsz##_shared_close_lpsz_(nullptr, [&](void *) {if (lpwsz != NULL) {::GlobalFree(lpwsz);lpwsz = 0; } });
// 释放进程句柄
#define hmc_shared_close_handle(handle) std::shared_ptr<void>##lpwsz##_shared_free_handle(nullptr, [&](void *) {if (handle != NULL) { try{::CloseHandle(handle);}catch(...){}} });

// 开辟内存 (请注意需要调用 FreeVS 或者 hmc_FreeVSAuto 释放)
// -> char* pszMem = AllocVS(char*, leng + 1);
#define hmc_AllocVS(Type, leng) \
    (Type)::VirtualAlloc((LPVOID)NULL, (DWORD)(leng), MEM_COMMIT, PAGE_READWRITE);

// 释放空间
// -> char* pszMem = hmc_AllocVS(char*, leng + 1);
// -> FreeVS(pszMem);
#define hmc_FreeVS(Virtua) \
    if (Virtua != NULL)    \
        ::VirtualFree(Virtua, 0, MEM_RELEASE);

// 释放空间
// -> char* pszMem = hmc_AllocVS(char*, leng + 1);
// -> FreeVSAuto(pszMem);
#define hmc_FreeVSAuto(Virtua) \
    std::shared_ptr<void>##Virtua##_shared_close_FreeVSAuto_(nullptr, [&](void *) {if (Virtua != NULL) {::VirtualFree(Virtua, 0, MEM_RELEASE);} });

// 开辟内存
#define hmc_AllocHeap(variable) ::HeapAlloc(::GetProcessHeap(), 0, (variable))
// 释放内存
#define hmc_FreeHeap(variable) ::HeapFree(::GetProcessHeap(), 0, (variable))

// 让代码在异步线程中执行
#define hmc_Thread (code) std::thread([]() -> void { code }).detach();

#define hmc_Catch                                                                                  \
    catch (char *e) { std::cout << "ERROR:"                                                        \
                                << "<" << __FUNCSIG__ << " @ " << __LINE__ << "> " << e << "\n"; } \
    catch (const std::exception &ex)                                                               \
    {                                                                                              \
        std::cout << "exception:"                                                                  \
                  << "<" << __FUNCSIG__ << " @ " << __LINE__ << "> " << ex.what() << "\n";         \
    }                                                                                              \
    catch (...) { std::cout << "ERROR:"                                                            \
                            << "<" << __FUNCSIG__ << " @ " << __LINE__ << "> "                     \
                            << "unknown"                                                           \
                            << "\n"; };

#define ___hmc_string_util_eq(types) (input.type() == typeid(types) ? true : false)
#define ___hmc_string_util_eqt(types) (input.type() == types ? true : false)
#define ___hmc_string_util_diff_any_value_eval_map_to_fn_map(any_value, fn_name, map_type, key_type, value_type) \
    if (any_value.type() == typeid(map_type<key_type, value_type>))                                              \
    {                                                                                                            \
        return fn_name(std::any_cast<map_type<key_type, value_type>>(any_value));                                \
    }                                                                                                            \
    if (any_value.type() == typeid(map_type<value_type, key_type>))                                              \
    {                                                                                                            \
        return fn_name(std::any_cast<map_type<value_type, key_type>>(any_value));                                \
    }

    }

};

namespace HMC_TEMPLATE_FUNC
{

    using T = std::wstring;
    // using FILTER_T = std::function<bool(wchar_t at, size_t index, T *input)>;

    template <typename T, typename T2>
    /**
     * @brief 数组/ 字符对字符 容器中包含此字符？
     *
     * @param _Vec 数组/string
     * @param _Char 字符/字符集
     * @return true
     * @return false
     */
    static bool VecContainChar(T _Vec, T2 _Char)
    {
        bool result = false;

        static_assert(
            std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string> ||
                std::is_same_v<T, std::vector<char>> || std::is_same_v<T, std::vector<wchar_t>> ||
                std::is_same_v<T, wchar_t> || std::is_same_v<T, char>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        static_assert(
            std::is_same_v<T2, wchar_t> ||
                std::is_same_v<T2, char> ||
                std::is_same_v<T2, std::vector<char>> ||
                std::is_same_v<T2, std::vector<wchar_t>>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        if constexpr (std::is_same_v<T, wchar_t> || std::is_same_v<T, char>)
        {
            const auto at = _Vec;

            if constexpr (std::is_same_v<T2, wchar_t> || std::is_same_v<T2, char>)
            {

                if (at == _Char)
                {
                    return true;
                }
            }

            if constexpr (std::is_same_v<T2, std::vector<char>> || std::is_same_v<T2, std::vector<wchar_t>>)
            {

                for (auto &&_filter : _Char)
                {

                    if (at == _filter)
                    {
                        return true;
                    }
                }
            }
        }

        if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::vector<char>> || std::is_same_v<T, std::vector<wchar_t>>)
        {
            for (size_t i = 0; i < _Vec.size(); i++)
            {
                auto &at = _Vec.at(i);

                if constexpr (std::is_same_v<T2, wchar_t> || std::is_same_v<T2, char>)
                {

                    if (at == _Char)
                    {
                        return true;
                    }
                }

                if constexpr (std::is_same_v<T2, std::vector<char>> || std::is_same_v<T2, std::vector<wchar_t>>)
                {

                    for (auto &&_filter : _Char)
                    {

                        if (at == _filter)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return result;
    }

    template <typename T, typename FILTER_T>
    static size_t removeMatchChar(T *StrInput, FILTER_T filter)
    {

        size_t result = 0;
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(
            std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index, T * input)>> ||
                std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index)>> ||
                std::is_same_v<FILTER_T, std::function<bool(wchar_t at)>> ||
                std::is_same_v<FILTER_T, std::function<bool(char at, size_t index, T *input)>> ||
                std::is_same_v<FILTER_T, std::function<bool(char at, size_t index)>> ||
                std::is_same_v<FILTER_T, std::function<bool(char at)>> ||
                std::is_same_v<FILTER_T, wchar_t> ||
                std::is_same_v<FILTER_T, char> ||
                std::is_same_v<FILTER_T, std::vector<char>> ||
                std::is_same_v<FILTER_T, std::vector<wchar_t>>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        if (StrInput == nullptr || StrInput->empty())
            return result;

        size_t the_size = StrInput->size();

        if constexpr (std::is_same_v<FILTER_T, wchar_t>)
        {
            StrInput->erase(std::remove(StrInput->begin(), StrInput->end(), filter),
                            StrInput->end());
            result = the_size - StrInput->size();
        }

        if constexpr (std::is_same_v<FILTER_T, char>)
        {
            StrInput->erase(std::remove(StrInput->begin(), StrInput->end(), filter),
                            StrInput->end());
            result = the_size - StrInput->size();
        }

        if constexpr (std::is_same_v<FILTER_T, std::vector<char>> || std::is_same_v<FILTER_T, std::vector<wchar_t>>)
        {
            size_t filter_index = StrInput->size();

            for (auto it = StrInput->rbegin(); it != StrInput->rend(); it++)
            {
                filter_index--;
                auto at = *it;
                bool is_Contain = false;

                if constexpr (std::is_same_v<FILTER_T, std::vector<char>>)
                {
                    is_Contain = VecContainChar<FILTER_T, char>(filter, at);
                }

                if constexpr (std::is_same_v<FILTER_T, std::vector<wchar_t>>)
                {
                    is_Contain = VecContainChar<FILTER_T, wchar_t>(filter, at);
                }

                if (is_Contain)
                {
                    if (filter_index < StrInput->size())
                    {
                        StrInput->erase(filter_index, 1);
                    }
                }
            }
        }

        if constexpr (
            std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index, T * input)>> ||
            std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index)>> ||
            std::is_same_v<FILTER_T, std::function<bool(wchar_t at)>>)
        {
            size_t filter_index = StrInput->size();

            for (auto it = StrInput->rbegin(); it != StrInput->rend(); it++)
            {
                filter_index--;
                auto at = *it;
                bool is_Contain = false;

                if constexpr (std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index, T * input)>>)
                {
                    if (filter == nullptr)
                        continue;
                    is_Contain = filter(at, filter_index, StrInput);
                }

                if constexpr (std::is_same_v<FILTER_T, std::function<bool(wchar_t at, size_t index)>>)
                {
                    if (filter == nullptr)
                        continue;
                    is_Contain = filter(at, filter_index);
                }

                if constexpr (std::is_same_v<FILTER_T, std::function<bool(wchar_t at)>>)
                {
                    if (filter == nullptr)
                        continue;
                    is_Contain = filter(at);
                }

                if (is_Contain)
                {
                    if (filter_index < StrInput->size())
                    {
                        StrInput->erase(filter_index, 1);
                    }
                }
            }
        }

        result = the_size - StrInput->size();
        return result;
    }

    template <typename T, typename T2>
    static T GetAsciiPrefix(const T input, T2 filter)
    {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<T2, std::vector<char>> || std::is_same_v<T2, std::monostate> || std::is_same_v<T2, char>, "Unsupported type preset escape (不支持的类型预设转义)");

        T result;
        if (input.empty())
            return result;
        result.reserve(input.size());
        for (size_t i = 0; i < input.size(); i++)
        {
            auto it = input.at(i);

            if constexpr (std::is_same_v<T2, std::vector<char>> || std::is_same_v<T2, char>)
            {

                bool is_continue = false;

                if constexpr (std::is_same_v<T2, std::vector<char>>)
                {
                    for (auto &&_filter : filter)
                    {

                        if (it == _filter)
                        {
                            is_continue = true;
                        }
                    }
                }

                if constexpr (std::is_same_v<T2, char>)
                {

                    if (it == filter)
                    {
                        is_continue = true;
                    }
                }

                if (is_continue)
                {
                    continue;
                }
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

    template <typename T, typename MATCH_T>
    static void split(const T &sourcePtr, MATCH_T splitter, std::vector<T> *item_list, const bool ignore_empty)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(
            // 字符匹配
            std::is_same_v<MATCH_T, wchar_t> ||
                std::is_same_v<MATCH_T, char> ||
                // 多字符匹配
                std::is_same_v<MATCH_T, std::vector<char>> ||
                std::is_same_v<MATCH_T, std::vector<wchar_t>> ||
                // 字符串匹配
                std::is_same_v<MATCH_T, std::string> ||
                std::is_same_v<MATCH_T, std::wstring>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        if (item_list == NULL)
        {
            return;
        }

        item_list->clear();
        if (sourcePtr.empty())
        {
            return;
        }

        // 字符匹配

        if constexpr (std::is_same_v<MATCH_T, wchar_t> || std::is_same_v<MATCH_T, char> || std::is_same_v<MATCH_T, std::vector<char>> || std::is_same_v<MATCH_T, std::vector<wchar_t>>)
        {

            T item;
            item.reserve(sourcePtr.length());

            bool is_Contain = false;

            for (auto &data : sourcePtr)
            {
                is_Contain = false;

                if constexpr (std::is_same_v<MATCH_T, wchar_t> || std::is_same_v<MATCH_T, char>)
                {
                    is_Contain = data == splitter;
                }

                if constexpr (std::is_same_v<MATCH_T, std::vector<char>> || std::is_same_v<MATCH_T, std::vector<wchar_t>>)
                {
                    is_Contain = VecContainChar<MATCH_T, wchar_t>(splitter, data);
                }

                if (is_Contain)

                {
                    if (!ignore_empty || !item.empty())
                    {
                        item_list->push_back(T(item.begin(), item.end()));
                        item.clear();
                    }
                }
                else
                {
                    item.push_back(data);
                }
            }

            if ((!ignore_empty && is_Contain) || !item.empty())
            {
                item.shrink_to_fit();
                item_list->push_back(std::move(item));
            }
        }

        if constexpr (std::is_same_v<MATCH_T, std::wstring> || std::is_same_v<MATCH_T, std::string>)
        {
            size_t start_pos = 0;
            size_t last_pos = 0;
            size_t length = 0;

            while (true)
            {
                start_pos = sourcePtr.find(splitter, last_pos);

                if (start_pos == sourcePtr.npos)
                {
                    start_pos = sourcePtr.size();
                }

                length = start_pos - last_pos;

                if (length >= (size_t)-500)
                {
                    break;
                }

                if (!ignore_empty || length != 0)
                {
                    item_list->push_back(sourcePtr.substr(last_pos, length));
                }

                if (start_pos >= sourcePtr.size())
                {
                    break;
                }
                else
                {
                    last_pos = start_pos + splitter.size();
                }
            }
        }
    }

    template <typename T, typename MATCH_T>
    static std::vector<T> split(const T &sourcePtr, MATCH_T splitter, const bool ignore_empty)
    {
        std::vector<T> item_list;
        if (sourcePtr.empty())
        {
            return item_list;
        }

        split<T, MATCH_T>(sourcePtr, splitter, &item_list, ignore_empty);
        return item_list;
    }

    template <typename T>
    static inline T text_to_lower_or_upper(T data, const bool is_upper)
    {
        static_assert(
            std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string> ||
                std::is_same_v<T, char> || std::is_same_v<T, wchar_t>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        T Result;

        using char_type = typename T::value_type;

        if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>)
        {

            for (char_type &c : data)

            {
                if (std::isalpha(static_cast<int>(c)))
                {
                    if (is_upper)
                        Result.push_back(std::toupper(c));
                    else
                    {
                        Result.push_back(std::tolower(c));
                    }
                }
                else
                {
                    Result.shrink_to_fit();
                    Result.push_back(c);
                }
            }
        }

        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, wchar_t>)
        {

            if (std::isalpha(static_cast<int>(data)))
            {
                if (is_upper)
                {
                    return std::toupper(data);
                }
                else
                {
                    return std::tolower(data);
                }
            }
        }

        return Result;
    }

    template <typename T>
    static inline size_t text_to_lower_or_upper(T *data, const bool is_upper)
    {

        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        using char_type = typename T::value_type;
        size_t result = 0;

        if (data == NULL)
        {
            return result;
        }

        if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>)
        {

            for (size_t index = 0; index < data->size(); index++)
            {
                char_type &at = data->at(index);

                if (std::isalpha(static_cast<int>(at)))
                {
                    if (is_upper)
                    {
                        data[index] = std::toupper(at);
                    }
                    else
                    {
                        data[index] = std::tolower(at);
                    }

                    result = result + 1;
                }
            }
        }
        return result;
    }

    template <typename T>
    static std::vector<T> convertMultiStringArray(const T input)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        std::vector<T> result = {};

        T temp = T();

        size_t size = input.size();

        for (size_t i = 0; i < size; i++)
        {
            auto &it = input.at(i);

            if (it == '\0')
            {
                // 最后两位如果是\0忽略它
                if (i + 2 < size)
                {
                    result.push_back(std::move(temp));
                    temp.clear();
                }

                continue;
            }
            temp.push_back(it);
        }

        if (!temp.empty())
        {
            result.push_back(std::move(temp));
        }

        return result;
    }

    template <typename T>
    static void convertMultiStringArray(const std::vector<T> input, T *result)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        if (!result)
        {
            return;
        }
        using char_type = typename T::value_type;

        if (input.empty())
        {
            result->push_back('\0');
            result->push_back('\0');
            return;
        }

        result->reserve(size<T>(input) + (input.size() * 2));

        for (size_t i = 0; i < input.size(); i++)
        {
            auto &at = input.at(i);
            T temp = T(at.begin(), at.end());
            removeMatchChar<T, char_type>(&temp, '\0');
            result->append(std::move(temp));
            result->push_back('\0');
        }

        result->push_back('\0');
        return;
    }

    template <typename T>
    static T convertMultiStringArray(const std::vector<T> input)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();

        convertMultiStringArray<T>(input, &result);

        // using char_type = typename T::value_type;

        // if (input.empty())
        //{
        //     result.push_back('\0');
        //     result.push_back('\0');
        //     return result;
        // }

        // result.reserve(size<T>(input) + (input.size() * 2));

        // for (size_t i = 0; i < input.size(); i++)
        //{
        //     auto &at = input.at(i);
        //     T temp = T(at.begin(), at.end());
        //     removeMatchChar<T, char>(&temp, '\0');
        //     result.append(std::move(temp));
        //     result.push_back('\0');
        // }

        // result.push_back('\0');
        return result;
    }

    template <typename T, typename SepT>
    static void join(const std::vector<T> input, SepT separator, T &result)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<SepT, std::monostate> || std::is_same_v<SepT, char> || std::is_same_v<SepT, wchar_t> | std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        size_t separator_size = 1;

        if constexpr (std::is_same_v<SepT, std::wstring> || std::is_same_v<SepT, std::string>)
        {
            separator_size = separator.size();
        }

        result.reserve(size(input) + (input.size() * separator_size));

        for (size_t i = 0; i < input.size(); i++)
        {
            auto &at = input.at(i);
            result.append(at.begin(), at.end());

            if constexpr (!std::is_same_v<SepT, std::monostate>)
            {

                if (i + 1 < input.size())
                {

                    if constexpr (std::is_same_v<SepT, std::wstring> || std::is_same_v<SepT, std::string>)
                    {
                        auto temp = T(separator.begin(), separator.end());
                        result.append(temp);
                    }

                    if constexpr (std::is_same_v<SepT, char> || std::is_same_v<SepT, wchar_t>)
                    {
                        result.push_back(separator);
                    }
                }
            }
        }
    }

    template <typename T, typename SepT>
    static T join(const std::vector<T> input, SepT separator)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<SepT, std::monostate> || std::is_same_v<SepT, char> || std::is_same_v<SepT, wchar_t> | std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        T result = T();
        join<T, SepT>(input, separator, result);
        return result;
    }

    template <typename T>
    static inline size_t size(const std::vector<T> input)
    {
        size_t result = 0;

        for (size_t i = 0; i < input.size(); i++)
        {
            auto &at = input.at(i);
            result = result + at.size();
        }

        return result;
    }

    template <typename T>
    static inline bool is_ascii(const T input)
    {
        bool result = false;
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string> || std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Unsupported type preset escape (不支持的类型预设转义)");

        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, wchar_t>)
        {
            if (input > 127)
            {
                return result;
            }
        }

        if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>)
        {

            for (auto &&item : input)
            {
                if (item > 127)
                {
                    return result;
                }
            }
        }

        result = true;
        return result;
    }

    template <typename T, typename char_type>
    static T lpstr_to_string(char_type *input, int nBufSize, bool earlyTruncation)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<char_type, wchar_t> || std::is_same_v<char_type, char>, "Unsupported type preset escape (不支持的类型预设转义)");

        // using char_type = typename T::value_type;
        T result = T();

        if (input == nullptr || nBufSize <= 0)
            return result;

        result.reserve(nBufSize);

        size_t length = static_cast<size_t>(nBufSize);

        for (size_t i = 0; i < length; i++)
        {
            if (input == NULL)
            {
                break;
            }

            char_type data = static_cast<char_type>(input[i]);

            if (earlyTruncation && data == '\0')
            {
                break;
            }
            result.push_back(data);
        }

        return result;
    }

    template <typename T, typename char_type>
    static T lpstr_to_string(char_type *input)
    {

        T result = T();
        if (input != nullptr)
        {
            char_type *current = input;
            while (*current != '\0')
            {
                result.push_back(*current);
                current++;
            }
        }
        return result;
    }

    template <typename T, typename char_type>
    static const char_type *formatCStr(T *input)
    {

        if (input == nullptr)
        {
            return nullptr;
        }

        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<char_type, wchar_t> || std::is_same_v<char_type, char>, "Unsupported type preset escape (不支持的类型预设转义)");

        removeMatchChar<T, char>(input, '\0');
        input->push_back('\0');
        return input->c_str();
    }

    template <typename T, typename char_type>
    static void formatCStr(T &input)
    {

        if (input.empty())
        {
            return;
        }

        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        removeMatchChar<T, char>(&input, '\0');
        input.push_back('\0');
    }

    template <typename T, typename char_type>
    static const char_type *formatCStr(const std::vector<T> input, T *temp_ptr)
    {

        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<char_type, wchar_t> || std::is_same_v<char_type, char>, "Unsupported type preset escape (不支持的类型预设转义)");

        convertMultiStringArray<T>(input, temp_ptr);
        return temp_ptr->c_str();
    }

    template <typename T, typename From>
    static inline size_t replace(T *sourcePtr, const From from, const From to, bool isAllReplace)
    {

        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");
        static_assert(std::is_same_v<From, char> || std::is_same_v<From, wchar_t> || std::is_same_v<From, std::wstring> || std::is_same_v<From, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        size_t result = 0;

        if (sourcePtr == nullptr)
        {
            return result;
        }

        if constexpr (std::is_same_v<From, std::wstring> || std::is_same_v<From, std::string>)
        {

            size_t start_pos = 0;

            while ((start_pos = sourcePtr->find(from, start_pos)) != sourcePtr->npos)
            {
                if (start_pos >= sourcePtr->size())
                {
                    return result;
                }

                sourcePtr->replace(start_pos, from.length(), to);

                start_pos += to.length();

                result = result + 1;
                if (!isAllReplace)
                {
                    break;
                }
            }
        }

        if constexpr (std::is_same_v<From, char> || std::is_same_v<From, wchar_t>)
        {
            size_t count = sourcePtr->size();

            for (size_t i = 0; i < count; i++)
            {
                auto &at = sourcePtr->at(i);
                if (at == from)
                {
                    (*sourcePtr)[i] = to;
                    result = result + 1;

                    if (!isAllReplace)
                    {
                        break;
                    }
                }
            }
        }

        return result;
    }

    template <typename T, typename From>
    static inline T asciiConvers(From input)
    {
        static_assert(std::is_same_v<T, std::wstring> || std::is_same_v<T, std::string>, "Unsupported type preset escape (不支持的类型预设转义)");

        static_assert(
                std::is_same_v<From, std::wstring>||
                std::is_same_v<From, std::string>||
                std::is_same_v<From, std::vector<char>>||
                std::is_same_v<From, std::vector<wchar_t>>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        using char_type = typename T::value_type;

        T result = T();

        result.reserve(input.size());

        for (auto &&item : input)
        {
            if (127 >= item && item >= 0)
            {
                result.push_back(static_cast<char_type>(item));
            }
        }

        return result;
    }

}

// * 模版化 转发

namespace hmc_string_util
{

    namespace HMC = HMC_TEMPLATE_FUNC;

    // std::wstring hmc_string_util::formatJson(std::wstring json, wchar_t indent, size_t indentSize) { return HMC::formatJson<std::wstring, wchar_t>(json, indent, indentSize); }
    // std::string hmc_string_util::formatJson(std::string json, char indent, size_t indentSize) { return HMC::formatJson<std::string, char>(json, indent, indentSize); }

    const char *hmc_string_util::formatCStr(std::string *input) { return HMC::formatCStr<std::string, char>(input); }
    const char *hmc_string_util::formatCStr(const std::vector<std::string> input, std::string *temp_ptr) { return HMC::formatCStr<std::string, char>(input, temp_ptr); }
    const wchar_t *hmc_string_util::formatCStr(std::wstring *input) { return HMC::formatCStr<std::wstring, wchar_t>(input); }
    const wchar_t *hmc_string_util::formatCStr(const std::vector<std::wstring> input, std::wstring *temp_ptr) { return HMC::formatCStr<std::wstring, wchar_t>(input, temp_ptr); }
    void hmc_string_util::formatCStr(std::string &input) { return HMC::formatCStr<std::string, char>(input); }
    void hmc_string_util::formatCStr(std::wstring &input) { return HMC::formatCStr<std::wstring, wchar_t>(input); }

    // 字符匹配模式
    size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, wchar_t filter) { return HMC::removeMatchChar<std::wstring, wchar_t>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::vector<wchar_t> filter) { return HMC::removeMatchChar<std::wstring, std::vector<wchar_t>>(StrInput, filter); }

    // 函数匹配
    size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index, std::wstring *input)> filter) { return HMC::removeMatchChar<std::wstring, std::function<bool(wchar_t at, size_t index, std::wstring * input)>>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index)> filter) { return HMC::removeMatchChar<std::wstring, std::function<bool(wchar_t at, size_t index)>>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at)> filter) { return HMC::removeMatchChar<std::wstring, std::function<bool(wchar_t at)>>(StrInput, filter); }

    // 字符匹配模式
    size_t hmc_string_util::removeMatchChar(std::string *StrInput, char filter) { return HMC::removeMatchChar<std::string, char>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::vector<char> filter) { return HMC::removeMatchChar<std::string, std::vector<char>>(StrInput, filter); }

    // 函数匹配
    size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index, std::string *input)> filter) { return HMC::removeMatchChar<std::string, std::function<bool(char at, size_t index, std::string *input)>>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index)> filter) { return HMC::removeMatchChar<std::string, std::function<bool(char at, size_t index)>>(StrInput, filter); }
    size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at)> filter) { return HMC::removeMatchChar<std::string, std::function<bool(char at)>>(StrInput, filter); }

    // ansi
    std::string hmc_string_util::getAsciiPrefix(const std::string &path, char filter) { return HMC::GetAsciiPrefix<std::string, char>(path, filter); }
    std::string hmc_string_util::getAsciiPrefix(const std::string &path, std::vector<char> filterList) { return HMC::GetAsciiPrefix<std::string, std::vector<char>>(path, filterList); }
    std::string hmc_string_util::getAsciiPrefix(const std::string &path) { return HMC::GetAsciiPrefix<std::string, std::monostate>(path, std::monostate()); }

    // 宽字符
    std::wstring hmc_string_util::getAsciiPrefix(const std::wstring &path) { return HMC::GetAsciiPrefix<std::wstring, std::monostate>(path, std::monostate()); }
    std::wstring hmc_string_util::getAsciiPrefix(const std::wstring &path, char filter) { return HMC::GetAsciiPrefix<std::wstring, char>(path, filter); }
    std::wstring hmc_string_util::getAsciiPrefix(const std::wstring &path, std::vector<char> filterList) { return HMC::GetAsciiPrefix<std::wstring, std::vector<char>>(path, filterList); }

    // ejson
    // inline std::string hmc_json_util::escapeJsonString(const std::string &input, bool is_to_value) { return HMC::escapeJsonString<std::string>(input, is_to_value); }
    // inline std::wstring hmc_json_util::escapeJsonString(const std::wstring &input, bool is_to_value) { return HMC::escapeJsonString<std::wstring>(input, is_to_value); }

    // 单字符
    void hmc_string_util::split(const std::string &sourcePtr, char splitter, std::vector<std::string> &item_list, const bool ignore_empty) { return HMC::split<std::string, char>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::string> hmc_string_util::split(const std::string &sourcePtr, char splitter, const bool ignore_empty) { return HMC::split<std::string, char>(sourcePtr, splitter, ignore_empty); }
    void hmc_string_util::split(const std::wstring &sourcePtr, wchar_t splitter, std::vector<std::wstring> &item_list, const bool ignore_empty) { return HMC::split<std::wstring, wchar_t>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::wstring> hmc_string_util::split(const std::wstring &sourcePtr, wchar_t splitter, const bool ignore_empty) { return HMC::split<std::wstring, wchar_t>(sourcePtr, splitter, ignore_empty); }

    // 单字符 多个匹配
    void hmc_string_util::split(const std::string &sourcePtr, std::vector<char> splitter, std::vector<std::string> &item_list, const bool ignore_empty) { return HMC::split<std::string, std::vector<char>>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::string> hmc_string_util::split(const std::string &sourcePtr, std::vector<char> splitter, const bool ignore_empty) { return HMC::split<std::string, std::vector<char>>(sourcePtr, splitter, ignore_empty); }
    void hmc_string_util::split(const std::wstring &sourcePtr, std::vector<wchar_t> splitter, std::vector<std::wstring> &item_list, const bool ignore_empty) { return HMC::split<std::wstring, std::vector<wchar_t>>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::wstring> hmc_string_util::split(const std::wstring &sourcePtr, std::vector<wchar_t> splitter, const bool ignore_empty) { return HMC::split<std::wstring, std::vector<wchar_t>>(sourcePtr, splitter, ignore_empty); }

    // 字符串
    void hmc_string_util::split(const std::string &sourcePtr, std::string splitter, std::vector<std::string> &item_list, const bool ignore_empty) { return HMC::split<std::string, std::string>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::string> hmc_string_util::split(const std::string &sourcePtr, std::string splitter, const bool ignore_empty) { return HMC::split<std::string, std::string>(sourcePtr, splitter, ignore_empty); }
    void hmc_string_util::split(const std::wstring &sourcePtr, std::wstring splitter, std::vector<std::wstring> &item_list, const bool ignore_empty) { return HMC::split<std::wstring, std::wstring>(sourcePtr, splitter, &item_list, ignore_empty); }
    std::vector<std::wstring> hmc_string_util::split(const std::wstring &sourcePtr, std::wstring splitter, const bool ignore_empty) { return HMC::split<std::wstring, std::wstring>(sourcePtr, splitter, ignore_empty); }

    // 字符串转大小写
    std::string hmc_string_util::text_to_upper(const std::string data) { return HMC::text_to_lower_or_upper<std::string>(data, true); };
    std::wstring hmc_string_util::text_to_upper(const std::wstring data) { return HMC::text_to_lower_or_upper<std::wstring>(data, true); };
    size_t hmc_string_util::text_to_upper(std::string *data) { return HMC::text_to_lower_or_upper<std::string>(data, true); };
    size_t hmc_string_util::text_to_upper(std::wstring *data) { return HMC::text_to_lower_or_upper<std::wstring>(data, true); };
    size_t hmc_string_util::text_to_lower(std::string *data) { return HMC::text_to_lower_or_upper<std::string>(data, false); };
    size_t hmc_string_util::text_to_lower(std::wstring *data) { return HMC::text_to_lower_or_upper<std::wstring>(data, false); };
    std::string hmc_string_util::text_to_lower(const std::string data) { return HMC::text_to_lower_or_upper<std::string>(data, false); };
    std::wstring hmc_string_util::text_to_lower(const std::wstring data) { return HMC::text_to_lower_or_upper<std::wstring>(data, false); };

    // 字符串转数组
    std::vector<std::wstring> hmc_string_util::convertMultiStringArray(const std::wstring input) { return HMC::convertMultiStringArray<std::wstring>(input); }
    std::vector<std::string> hmc_string_util::convertMultiStringArray(const std::string input) { return HMC::convertMultiStringArray<std::string>(input); }
    std::wstring hmc_string_util::convertMultiStringArray(const std::vector<std::wstring> input) { return HMC::convertMultiStringArray<std::wstring>(input); }
    std::string hmc_string_util::convertMultiStringArray(const std::vector<std::string> input) { return HMC::convertMultiStringArray<std::string>(input); }

    // 是否是 ascii
    inline bool hmc_string_util::is_ascii(const std::wstring input) { return HMC::is_ascii<std::wstring>(input); }
    inline bool hmc_string_util::is_ascii(const std::string input) { return HMC::is_ascii<std::string>(input); }
    inline bool hmc_string_util::is_ascii(char Input) { return HMC::is_ascii<char>(Input); }
    inline bool hmc_string_util::is_ascii(wchar_t Input) { return HMC::is_ascii<wchar_t>(Input); }

    std::string hmc_string_util::join(const std::vector<std::string> &item_list) { return HMC::join<std::string, std::monostate>(item_list, std::monostate()); }
    std::string hmc_string_util::join(const std::vector<std::string> &item_list, std::string splitter) { return HMC::join<std::string, std::string>(item_list, splitter); }
    void hmc_string_util::join(const std::vector<std::string> &item_list, std::string splitter, std::string &outputPtr) { return HMC::join<std::string, std::string>(item_list, splitter, outputPtr); }
    std::string hmc_string_util::join(const std::vector<std::string> input, char separator) { return HMC::join<std::string, char>(input, separator); }
    std::wstring hmc_string_util::join(const std::vector<std::wstring> &item_list) { return HMC::join<std::wstring, std::monostate>(item_list, std::monostate()); }
    std::wstring hmc_string_util::join(const std::vector<std::wstring> &item_list, std::wstring splitter) { return HMC::join<std::wstring, std::wstring>(item_list, splitter); }
    void hmc_string_util::join(const std::vector<std::wstring> &item_list, std::wstring splitter, std::wstring &outputPtr) { return HMC::join<std::wstring, std::wstring>(item_list, splitter, outputPtr); }
    std::wstring hmc_string_util::join(const std::vector<std::wstring> input, wchar_t separator) { return HMC::join<std::wstring, wchar_t>(input, separator); }

    // 数组总字符大小
    inline size_t hmc_string_util::size(const std::vector<std::string> input) { return HMC::size<std::string>(input); }
    inline size_t hmc_string_util::size(const std::vector<std::wstring> input) { return HMC::size<std::wstring>(input); }

    std::wstring hmc_string_util::lpstr_to_string(wchar_t *input, int nBufSize, bool earlyTruncation) { return HMC::lpstr_to_string<std::wstring, wchar_t>(input, nBufSize, earlyTruncation); }
    std::string hmc_string_util::lpstr_to_string(char *input, int nBufSize, bool earlyTruncation) { return HMC::lpstr_to_string<std::string, char>(input, nBufSize, earlyTruncation); }

    std::wstring hmc_string_util::lpstr_to_string(wchar_t *input) { return HMC::lpstr_to_string<std::wstring, wchar_t>(input); }
    std::string hmc_string_util::lpstr_to_string(char *input) { return HMC::lpstr_to_string<std::string, char>(input); }

    inline std::string hmc_string_util::asciiConvers(std::wstring input) { return HMC::asciiConvers<std::string, std::wstring>(input); }
    inline std::wstring hmc_string_util::asciiConvers(std::string input) { return HMC::asciiConvers<std::wstring, std::string>(input); }
    inline std::string hmc_string_util::asciiConvers(std::vector<wchar_t> input) { return HMC::asciiConvers<std::string, std::vector<wchar_t>>(input); }
    inline std::wstring hmc_string_util::asciiConvers(std::vector<char> input) { return HMC::asciiConvers<std::wstring, std::vector<char>>(input); }

}

// 文本中是否有数字 并且是否是安全的 int32
bool hmc_string_util::is_int_str(const std::string Value)
{
    bool Result = false;
    if (Value.empty())
        return Result;
    try
    {
        int n = std::stoi(Value);
        Result = true;
    }
    catch (...)
    {
        return Result;
    }

    return Result;
}

// 文本中是否有数字 并且是否是安全的 long
bool hmc_string_util::is_long_str(const std::string Value)
{
    bool Result = false;
    if (Value.empty())
        return Result;
    try
    {
        long n = std::stol(Value);
        Result = true;
    }
    catch (...)
    {
        return Result;
    }

    return Result;
}

// 文本中是否有数字 并且是否是安全的 long long
bool hmc_string_util::is_longlong_str(const std::string Value)
{
    bool Result = false;
    if (Value.empty())
        return Result;
    try
    {
        long long n = std::stoll(Value);
        Result = true;
    }
    catch (...)
    {
        return Result;
    }

    return Result;
}

inline std::wstring hmc_string_util::utf8_to_utf16(const std::string input)
{
    std::wstring result;

    if (input.empty())
    {
        return result;
    }

    int inputSize = static_cast<UINT32>(input.size());

    int iSizeInChars = ::MultiByteToWideChar(CP_UTF8, 0, input.c_str(), inputSize, NULL, 0);

    if (iSizeInChars > 0)
    {
        size_t char_size = static_cast<size_t>(iSizeInChars);
        std::vector<wchar_t> TempResult = std::vector<wchar_t>(char_size, '\0');

        ::MultiByteToWideChar(CP_UTF8, 0, input.c_str(), inputSize, &TempResult[0], static_cast<int>(char_size));

        result.reserve(char_size + 5);
        result.append(TempResult.begin(), TempResult.end());
        if (!result.empty() && result.back() != '\0')
        {
            result.push_back('\0');
        }
    }

    return result;
}

inline std::string hmc_string_util::utf16_to_utf8(const std::wstring input)
{
    std::string result;

    if (input.empty())
    {
        return result;
    }

    int inputSize = static_cast<UINT32>(input.size());

    int iSizeInBytes = ::WideCharToMultiByte(CP_UTF8, 0, input.c_str(), inputSize, NULL, 0, NULL, NULL);

    if (iSizeInBytes > 0)
    {
        size_t char_size = static_cast<size_t>(iSizeInBytes);

        std::vector<char> TempResult = std::vector<char>(char_size, '\0');

        ::WideCharToMultiByte(CP_UTF8, 0, input.c_str(), inputSize, &TempResult[0], static_cast<int>(char_size), NULL, NULL);

        result.reserve(char_size + 5);
        result.append(TempResult.begin(), TempResult.end());
        if (!result.empty() && result.back() != '\0')
        {
            result.push_back('\0');
        }
    }

    return result;
}

inline std::string hmc_string_util::utf16_to_ansi(const std::wstring input)
{
    std::string result;

    if (input.empty())
    {
        return result;
    }

    int inputSize = static_cast<UINT32>(input.size());

    int iSizeInBytes = ::WideCharToMultiByte(CP_ACP, 0, input.c_str(), inputSize, NULL, 0, NULL, NULL);

    if (iSizeInBytes > 0)
    {
        std::vector<char> TempResult = std::vector<char>(iSizeInBytes, '\0');

        ::WideCharToMultiByte(CP_ACP, 0, input.c_str(), inputSize, &TempResult[0], iSizeInBytes, NULL, NULL);

        result.reserve(static_cast<int64_t>(iSizeInBytes) + 5);
        result.append(TempResult.begin(), TempResult.end());
        if (!result.empty() && result.back() != '\0')
        {
            result.push_back('\0');
        }
    }

    return result;
}

inline std::wstring hmc_string_util::ansi_to_utf16(const std::string input)
{
    std::wstring result;

    if (input.empty())
    {
        return result;
    }

    int inputSize = static_cast<UINT32>(input.size());

    int iSizeInChars = ::MultiByteToWideChar(CP_ACP, 0, input.c_str(), inputSize, NULL, 0);

    if (iSizeInChars > 0)
    {
        size_t char_size = static_cast<size_t>(iSizeInChars);

        std::vector<wchar_t> TempResult = std::vector<wchar_t>(char_size, '\0');

        ::MultiByteToWideChar(CP_ACP, 0, input.c_str(), inputSize, &TempResult[0], static_cast<int>(char_size));

        result.reserve(char_size + 5);
        result.append(TempResult.begin(), TempResult.end());
        if (!result.empty() && result.back() != '\0')
        {
            result.push_back('\0');
        }
    }

    return result;
}

// ----------------------------------------------------------------
// A2U8
// Local CP_ACP ( Ansi [A] ) to utf8 encoding
// winapi A字符 转 utf8(win 标准[W])
// ? 推荐w字符(utf16) 损耗更少一点 napi支持直接返回utf16
// ----------------------------------------------------------------
std::string hmc_string_util::ansi_to_utf8(const std::string ansi)
{
    std::wstring to_utf16 = ansi_to_utf16(ansi);
    return utf16_to_utf8(to_utf16);
    return std::string("");
}

// ----------------------------------------------------------------
// U82A
// utf8 to Local CP_ACP ( Ansi [A] ) encoding
// utf8 字符 转 winapi A字符
// ? 推荐w字符(utf16) 损耗更少一点 napi支持直接返回utf16
// ----------------------------------------------------------------
std::string hmc_string_util::utf8_to_ansi(const std::string utf8)
{
    std::wstring to_utf16 = utf8_to_utf16(utf8);
    return utf16_to_ansi(to_utf16);
}

inline bool hmc_string_util::is_utf8(const std::string input)
{
    for (std::string::const_iterator str = input.begin(), eos = input.end(); str != eos; ++str)
    {
        std::uint8_t cuChar = static_cast<std::uint8_t>(*str);
        size_t len = sizeof(cuChar);
        if (cuChar == '\b' || cuChar == '\n' || cuChar == '\r' || cuChar == '\f' || cuChar == '\t')
        {
            // 转义符
        }
        // 超出utf8的大小了
        else if (static_cast<std::uint8_t>(cuChar) >= 0xF8)
            return false;

        else if (static_cast<std::uint8_t>(cuChar) >= 0xF0)
        {
            if (len < 4)
                return false;
            for (int i = 1; i <= 3; i++)
            {
                std::uint8_t diff_cuChar = (cuChar + i);

                if ((diff_cuChar & 0xC0) != 0x80)
                    return false;
            }
            return true;
        }
        else if (static_cast<std::uint8_t>(cuChar) >= 0xE0)
        {
            if (len < 3)
                return false;
            for (int i = 1; i <= 2; i++)
            {
                std::uint8_t diff_cuChar = (cuChar + i);

                if ((diff_cuChar & 0xC0) != 0x80)
                    return false;
            }
            return true;
        }
        else if (static_cast<std::uint8_t>(cuChar) >= 0xC0)
        {

            if (len < 2)
                return false;
            std::uint8_t diff_cuChar = (cuChar + 1);

            if ((diff_cuChar & 0xC0) != 0x80)
                return false;
            return true;
        }
        else if (static_cast<std::uint8_t>(cuChar) >= 0x80)
            return false;
        else
            return true;
    }
    return true;
}

/**
 * @brief 全部替换
 *
 * @param sourcePtr
 * @param from
 * @param to
 */
inline void hmc_string_util::replaceAll(std::string &sourcePtr, std::string from, std::string to)
{
    size_t start_pos = 0;
    while ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        if (start_pos >= sourcePtr.size())
        {
            return;
        }
        sourcePtr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline void hmc_string_util::replaceAll(std::wstring &sourcePtr, const std::wstring from, const std::wstring to)
{
    size_t start_pos = 0;
    while ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        if (start_pos >= sourcePtr.size())
        {
            return;
        }
        sourcePtr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline void hmc_string_util::replaceAll(std::string &sourcePtr, std::vector<std::tuple<std::string, std::string>> list)
{

    size_t len = list.size();

    for (size_t i = 0; i < len; i++)
    {

        std::string from = std::get<0>(list[i]);
        std::string to = std::get<1>(list[i]);

        size_t start_pos = 0;
        while ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
        {
            if (start_pos >= sourcePtr.size())
            {
                return;
            }
            sourcePtr.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
}

inline void hmc_string_util::replaceAll(std::wstring &sourcePtr, std::vector<std::tuple<std::wstring, std::wstring>> list)
{
    size_t len = list.size();

    for (size_t i = 0; i < len; i++)
    {

        std::wstring from = std::get<0>(list[i]);
        std::wstring to = std::get<1>(list[i]);

        size_t start_pos = 0;
        while ((start_pos = sourcePtr.find(from, start_pos)) != std::wstring::npos)
        {
            if (start_pos >= sourcePtr.size())
            {
                return;
            }
            sourcePtr.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
}

/**
 * @brief 替换一次
 *
 * @param sourcePtr
 * @param from
 * @param to
 */
inline void hmc_string_util::replace(std::string &sourcePtr, std::string from, std::string to)
{
    size_t start_pos = 0;
    if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        if (start_pos >= sourcePtr.size())
        {
            return;
        }
        sourcePtr.replace(start_pos, from.length(), to);
        return;
    }
}

inline void hmc_string_util::replace(std::wstring &sourcePtr, const std::wstring from, const std::wstring to)
{
    size_t start_pos = 0;
    if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        if (start_pos >= sourcePtr.size())
        {
            return;
        }
        sourcePtr.replace(start_pos, from.length(), to);
        return;
    }
}

/**
 * @brief 替换一次
 *
 * @param sourcePtr
 * @param from
 * @param to
 */
inline void hmc_string_util::replace(std::string &sourcePtr, std::vector<std::tuple<std::string, std::string>> list)
{
    size_t len = list.size();

    for (size_t i = 0; i < len; i++)
    {

        std::string from = std::get<0>(list[i]);
        std::string to = std::get<1>(list[i]);
        size_t start_pos = 0;
        if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
        {
            if (start_pos >= sourcePtr.size())
            {
                return;
            }
            sourcePtr.replace(start_pos, from.length(), to);
            continue;
        }
    }
}

inline void hmc_string_util::replace(std::wstring &sourcePtr, std::vector<std::tuple<std::wstring, std::wstring>> list)
{
    size_t len = list.size();

    for (size_t i = 0; i < len; i++)
    {

        std::wstring from = std::get<0>(list[i]);
        std::wstring to = std::get<1>(list[i]);
        size_t start_pos = 0;
        if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
        {
            if (start_pos >= sourcePtr.size())
            {
                return;
            }
            sourcePtr.replace(start_pos, from.length(), to);
            continue;
        }
    }
}

/**
 * @brief 替换指定内容 仅在第N次出现的时候执行替换
 *
 * @param sourcePtr
 * @param from
 * @param bubble_index
 * @param to
 */
inline void hmc_string_util::replace(std::string &sourcePtr, std::string from, size_t bubble_index, std::string to)
{
    size_t start_pos = 0;
    size_t index = 0;
    if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        if (start_pos >= sourcePtr.size())
        {
            return;
        }
        if (index == bubble_index)
        {
            sourcePtr.replace(start_pos, from.length(), to);
            return;
        }
        index++;
    }
}

std::string hmc_string_util::trim(const std::string &input, const std::string &match)
{

    std::string temp_trim_str = trimLast(input, match);
    return trimFirst(temp_trim_str, match);
}

std::string hmc_string_util::trimFirst(const std::string &input, const std::string &match)
{
    auto start_pos = input.find_first_not_of(match);
    return (start_pos == std::string::npos) ? "" : input.substr(start_pos);
}

std::string hmc_string_util::trimLast(const std::string &input, const std::string &match)
{
    auto end_pos = input.find_last_not_of(match);
    return (end_pos == std::string::npos) ? "" : input.substr(0, end_pos + 1);
}

std::wstring hmc_string_util::trim(const std::wstring &input, const std::wstring &match)
{
    std::wstring temp_trim_str = trimLast(input, match);
    return trimFirst(temp_trim_str, match);
}

std::wstring hmc_string_util::trimFirst(const std::wstring &input, const std::wstring &match)
{
    auto start_pos = input.find_first_not_of(match);
    return (start_pos == std::string::npos) ? L"" : input.substr(start_pos);
}

std::wstring hmc_string_util::trimLast(const std::wstring &input, const std::wstring &match)
{
    auto end_pos = input.find_last_not_of(match);
    return (end_pos == std::string::npos) ? L"" : input.substr(0, end_pos + 1);
}

// 移除尾部 为xx 的指定文本
std::string hmc_string_util::trimLastAll(const std::string &input, const std::string &match)
{
    std::string result = std::string(input);
    if (match.empty())
    {
        return result;
    }
    for (size_t i = 0; i < input.size(); i++)
    {
        std::string ResultTemp = trimLast(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

// 移除开头和尾部 为xx 的指定文本
std::string hmc_string_util::trimAll(const std::string &input, const std::string &match)
{
    std::string result = std::string(input);
    if (match.empty())
    {
        return result;
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        std::string ResultTemp = trim(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

// 移除开头为xx 的指定文本
std::string hmc_string_util::trimFirstAll(const std::string &input, const std::string &match)
{
    std::string result = std::string(input);
    if (match.empty())
    {
        return result;
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        std::string ResultTemp = trimFirst(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

// 移除尾部 为xx 的指定文本
std::wstring hmc_string_util::trimAll(const std::wstring &input, const std::wstring &match)
{
    std::wstring result = std::wstring(input);
    if (match.empty())
    {
        return result;
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        std::wstring ResultTemp = trim(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

// 移除开头和尾部 为xx 的指定文本
std::wstring hmc_string_util::trimLastAll(const std::wstring &input, const std::wstring &match)
{
    std::wstring result = std::wstring(input);
    if (match.empty())
    {
        return result;
    }
    for (size_t i = 0; i < input.size(); i++)
    {
        std::wstring ResultTemp = trimLast(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

// 移除开头为xx 的指定文本
std::wstring hmc_string_util::trimFirstAll(const std::wstring &input, const std::wstring &match)
{
    std::wstring result = std::wstring(input);
    if (match.empty())
    {
        return result;
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        std::wstring ResultTemp = trimFirst(result, match);
        if (result.empty() || result.size() == ResultTemp.size())
        {
            return ResultTemp;
        }
        result.clear();
        result.append(ResultTemp);
    }

    return result;
}

std::wstring hmc_string_util::StripSpaces(const std::wstring &str)
{
    const std::size_t s = str.find_first_not_of(L" \t\r\n");

    if (str.npos != s)
        return str.substr(s, str.find_last_not_of(L" \t\r\n") - s + 1);
    else
        return str;
}

std::string hmc_string_util::StripSpaces(const std::string &str)
{
    const std::size_t s = str.find_first_not_of(" \t\r\n");

    if (str.npos != s)
        return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
    else
        return str;
}

std::vector<std::wstring> hmc_string_util::ansi_to_utf16(std::vector<std::string> item_list)
{
    std::vector<std::wstring> result = {};
    for (auto item : item_list)
    {
        result.push_back(ansi_to_utf16(item));
    }
    return result;
}

std::vector<std::string> hmc_string_util::utf16_to_ansi(std::vector<std::wstring> item_list)
{
    std::vector<std::string> result = {};
    for (auto item : item_list)
    {
        result.push_back(utf16_to_ansi(item));
    }
    return result;
}

std::map<std::wstring, std::wstring> hmc_string_util::ansi_to_utf16(std::map<std::string, std::string> item_list)
{
    std::map<std::wstring, std::wstring> result = {};
    for (auto item : item_list)
    {
        std::wstring key = ansi_to_utf16(item.first);
        std::wstring value = ansi_to_utf16(item.second);

        result.insert(std::make_pair(key, value));
    }
    return result;
}

std::map<std::string, std::string> hmc_string_util::utf16_to_ansi(std::map<std::wstring, std::wstring> item_list)
{
    std::map<std::string, std::string> result = {};
    for (auto item : item_list)
    {
        std::string key = utf16_to_ansi(item.first);
        std::string value = utf16_to_ansi(item.second);

        result.insert(std::make_pair(key, value));
    }
    return result;
}

const char *hmc_string_util::string_to_lpstr(std::string input, size_t *psize)
{

    std::string copy_input = std::string(input.begin(), input.end());

    copy_input.erase(std::remove(copy_input.begin(), copy_input.end(), '\0'),
                     copy_input.end());

    char *output = new (std::nothrow) char[copy_input.size() + sizeof(char)];

    if (output == nullptr)
    {
        return output;
    }

    for (size_t i = 0; i < copy_input.size(); i++)
    {
        char data = copy_input[i];
        output[i] = data;
    }
    const size_t end = copy_input.size();
    // 正好是 output-1 的位置
    output[end] = '\0';
    (*psize) = end;
    return output;
}

wchar_t *hmc_string_util::string_to_lpstr(std::wstring input, size_t *psize)
{

    std::wstring copy_input = std::wstring(input.begin(), input.end());

    copy_input.erase(std::remove(copy_input.begin(), copy_input.end(), L'\0'),
                     copy_input.end());

    wchar_t *output = new (std::nothrow) wchar_t[copy_input.size() + sizeof(wchar_t)];

    if (output == nullptr)
    {
        return output;
    }

    for (size_t i = 0; i < copy_input.size(); i++)
    {
        wchar_t data = copy_input[i];
        output[i] = data;
    }
    const size_t end = copy_input.size();
    // 正好是 output-1 的位置
    output[end] = L'\0';
    (*psize) = end;
    return output;
}

wchar_t *hmc_string_util::string_to_lpstr(std::vector<std::wstring> input_list, size_t *psize)
{

    std::wstring copy_input_list;

    const size_t count = input_list.size();

    size_t temp = 0;

    for (size_t i = 0; i < count; i++)
    {
        auto it = string_to_lpstr(input_list[i], &temp);
        copy_input_list.append(it);

        if (copy_input_list.back() != L'\0')
        {
            copy_input_list.push_back(L'\0');
        }
    }

    copy_input_list.push_back(L'\0');

    size_t len = copy_input_list.size();
    wchar_t *output = new (std::nothrow) wchar_t[len];

    if (output == nullptr)
    {
        return output;
    }

    for (size_t i = 0; i < len; i++)
    {
        output[i] = copy_input_list[i];
    }

    (*psize) = len;

    return output;
}

const char *hmc_string_util::string_to_lpstr(std::vector<std::string> input_list, size_t *psize)
{

    std::string copy_input_list;

    const size_t count = input_list.size();

    size_t temp = 0;

    for (size_t i = 0; i < count; i++)
    {
        auto it = string_to_lpstr(input_list[i], &temp);
        copy_input_list.append(it);

        if (copy_input_list.back() != '\0')
        {
            copy_input_list.push_back('\0');
        }
    }

    copy_input_list.push_back('\0');

    size_t len = copy_input_list.size();
    char *output = new (std::nothrow) char[len];

    if (output == nullptr)
    {
        return output;
    }

    for (size_t i = 0; i < len; i++)
    {
        output[i] = copy_input_list[i];
    }

    (*psize) = len;

    return output;
}

const char *hmc_string_util::string_to_lpstr(std::string input)
{
    size_t psize = 0;
    return string_to_lpstr(input, &psize);
}

wchar_t *hmc_string_util::string_to_lpstr(std::wstring input)
{

    size_t psize = 0;
    return string_to_lpstr(input, &psize);
}

/**
 * @brief 一个空的json 数组文本
 *
 * @return string
 */
std::string hmc_string_util::vec_to_array_json()
{
    std::string result = std::string("[]");
    return result;
}

/**
 * @brief cpp内容数组转为json的array文本
 *
 * @tparam T
 * @param data_list
 * @return string
 */
std::wstring hmc_string_util::vec_to_array_json(std::vector<std::wstring> item_list)
{
    std::wstring result = std::wstring(L"[");

    for (size_t i = 0; i < item_list.size(); i++)
    {
        auto data = item_list[i];

        std::wstring output = hmc_json_util::escapeJsonString(static_cast<std::wstring>(data), true);
        result.append(output);

        if (item_list.size() - 1 > i)
        {
            result.append(L",");
        }
    }
    result.append(L"]");
    return result;
}

#if _HAS_CXX17

/**
 * @brief c内容转为json的value
 *
 * @tparam T
 * @tparam V
 * @param value
 * @return string
 */
template <typename T, typename V>
std::string hmc_string_util::to_json_value(T &value)
{
    std::string result = "";
    static_assert(
        std::is_integral<T>::value ||
            std::is_floating_point<T>::value ||
            std::is_same_v<char, T> ||
            std::is_same_v<char *, T> ||
            std::is_same_v<const char *, T> ||
            std::is_same_v<std::vector<V>, T> ||
            std::is_same_v<T, std::string>,
        "Unsupported type preset escape (不支持的类型预设转义)");

    // boolean
    if constexpr (std::is_same_v<T, bool>)
    {
        result.append((value ? "true" : "false"));
    }
    // number -> int
    else if constexpr (std::is_integral<T>::value)
    {
        result.append(std::to_string(value));
    }
    // number -> float
    else if constexpr (std::is_floating_point<T>::value)
    {
        result.append(std::to_string(value));
    }
    // array -> <any>[]
    else if constexpr (std::is_same<std::vector<V>, T>::value)
    {
        result.append(hmc_string_util::vec_to_array_json(value));
    }
    // string -> "<char*>"
    else if constexpr (std::is_same_v<char, T> || std::is_same_v<char *, T> || std::is_same_v<const char *, T> || std::is_same_v<T, std::string>)
    {
        result.append("\"");
        std::string output = hmc_json_util::escapeJsonString(static_cast<std::string>(value));
        result.append(output);
        result.append("\"");
    }
    // null
    else
    {
        result.append("null");
    }

    return result;
}

template <typename KEY, typename V>
std::wstring hmc_string_util::map_to_jsonW(std::map<KEY, V> item_list)
{
    std::wstring result = L"{";

    static_assert(
        std::is_integral<KEY>::value ||
            std::is_floating_point<KEY>::value ||
            std::is_same_v<wchar_t, KEY> ||
            std::is_same_v<wchar_t *, KEY> ||
            std::is_same_v<const wchar_t *, KEY> ||
            std::is_same_v<KEY, std::wstring>,
        "Unsupported type preset escape (不支持的类型预设转义)");

    static_assert(
        std::is_integral<V>::value ||
            std::is_floating_point<V>::value ||
            std::is_same_v<wchar_t, V> ||
            std::is_same_v<wchar_t *, V> ||
            std::is_same_v<const wchar_t *, V> ||
            std::is_same_v<V, std::wstring>,
        "Unsupported type preset escape (不支持的类型预设转义)");
    size_t index = 0;

    for (auto item : item_list)
    {

        // 解析 key

        if constexpr (std::is_same_v<KEY, bool>)
        {
            result.append((item.first ? L"true" : L"false"));
        }
        // number -> int
        else if constexpr (std::is_integral<KEY>::value)
        {
            result.append(std::to_wstring(item.first));
        }
        // number -> float
        else if constexpr (std::is_floating_point<KEY>::value)
        {
            result.append(std::to_wstring(item.first));
        }

        else if constexpr (std::is_same_v<wchar_t, KEY> || std::is_same_v<wchar_t *, KEY> || std::is_same_v<const wchar_t *, KEY> || std::is_same_v<KEY, std::wstring>)
        {
            std::wstring output = hmc_json_util::escapeJsonString(static_cast<std::wstring>(item.first), true);
            result.append(output);
        }
        // null
        else
        {
            result.append(std::to_wstring(index));
        }

        result.push_back(L':');

        // result.append(hmc_string_util::to_json_value(item.second));

        if constexpr (std::is_same_v<V, bool>)
        {
            result.append((item.second ? L"true" : L"false"));
        }
        // number -> int
        else if constexpr (std::is_integral<V>::value)
        {
            result.append(std::to_wstring(item.second));
        }
        // number -> float
        else if constexpr (std::is_floating_point<V>::value)
        {
            result.append(std::to_wstring(item.second));
        }
        // string -> "<char*>"
        else if constexpr (std::is_same_v<wchar_t, V> || std::is_same_v<wchar_t *, V> || std::is_same_v<const wchar_t *, V> || std::is_same_v<V, std::wstring>)
        {
            std::wstring output = hmc_json_util::escapeJsonString(static_cast<std::wstring>(item.second), true);
            result.append(output);
        }
        // null
        else
        {
            result.append(L"null");
        }

        if (item_list.size() - 1 > index)
        {
            result.push_back(L',');
        }
        index++;
    }

    result.push_back(L'}');

    return result;
}

template <typename KEY, typename V>
std::string hmc_string_util::map_to_jsonA(std::map<KEY, V> item_list)
{
    std::string result = "{";

    static_assert(
        std::is_integral<KEY>::value ||
            std::is_floating_point<KEY>::value ||
            std::is_same_v<char, KEY> ||
            std::is_same_v<char *, KEY> ||
            std::is_same_v<const char *, KEY> ||
            std::is_same_v<KEY, std::string>,
        "Unsupported type preset escape (不支持的类型预设转义)");

    size_t index = 0;

    for (auto item : item_list)
    {

        // 解析 key

        if constexpr (std::is_same_v<KEY, bool>)
        {
            result.append((item.first ? "true" : "false"));
        }
        // number -> int
        else if constexpr (std::is_integral<KEY>::value)
        {
            result.append(std::to_string(item.first));
        }
        // number -> float
        else if constexpr (std::is_floating_point<KEY>::value)
        {
            result.append(std::to_string(item.first));
        }
        //// array -> <any>[]
        // else if constexpr (is_same<std::vector<ARR_V>, KEY>::value)
        //{
        //     result.append(vec_to_array_json(value));
        // }
        //  string -> "<char*>"
        else if constexpr (std::is_same_v<char, KEY> || std::is_same_v<char *, KEY> || std::is_same_v<const char *, KEY> || std::is_same_v<KEY, std::string>)
        {
            result.append("\"");

            std::string output = hmc_json_util::escapeJsonString(item.first);
            result.append(output);
            result.append("\"");
        }
        // null
        else
        {
            result.append(std::to_string(index));
        }

        result.push_back(':');

        // result.append(hmc_string_util::to_json_value(item.second));

        if constexpr (std::is_same_v<V, bool>)
        {
            result.append((item.second ? "true" : "false"));
        }
        // number -> int
        else if constexpr (std::is_integral<V>::value)
        {
            result.append(std::to_string(item.second));
        }
        // number -> float
        else if constexpr (std::is_floating_point<V>::value)
        {
            result.append(to_string(item.second));
        }
        // string -> "<char*>"
        else if constexpr (std::is_same_v<char, V> || std::is_same_v<char *, V> || std::is_same_v<const char *, V> || std::is_same_v<V, std::string>)
        {
            result.append("\"");

            std::string output = hmc_json_util::escapeJsonString(item.second);
            result.append(output);
            result.append("\"");
        }

        // null
        else
        {
            result.append("nul");
        }

        if (item_list.size() - 1 > index)
        {
            result.push_back(',');
        }
        index++;
    }

    result.push_back('}');

    return result;
}

/**
 * @brief cpp内容数组转为json的array文本
 *
 * @tparam T
 * @param data_list
 * @return string
 */
template <typename T>
std::string hmc_string_util::vec_to_array_json_any(const std::vector<T> data_list)
{
    std::string result = "[";

    static_assert(
        std::is_integral<T>::value ||
            std::is_floating_point<T>::value ||
            std::is_same_v<char, T> ||
            std::is_same_v<char *, T> ||
            std::is_same_v<const char *, T> ||
            std::is_same_v<T, std::string>,
        "Unsupported type preset escape (不支持的类型预设转义)");

    for (size_t index = 0; index < data_list.size(); index++)
    {
        auto value = data_list[index];

        if constexpr (std::is_same_v<T, bool>)
        {
            result.append((value ? "true" : "false"));
        }
        else if constexpr (std::is_integral<T>::value)
        {
            result.append(std::to_string(value));
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            result.append(std::to_string(value));
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            result.append(std::to_string(value));
        }
        else if constexpr (std::is_same_v<char, T> || std::is_same_v<char *, T> || std::is_same_v<const char *, T> || std::is_same_v<T, std::string>)
        {
            result.append("\"");
            std::string output = hmc_json_util::escapeJsonString(static_cast<std::string>(value));

            result.append(output);
            result.append("\"");
        }
        else
        {
            result.append("null");
        }

        if (data_list.size() - 1 > index)
        {
            result.append(",");
        }
    }

    result.append("]");

    return result;
}

std::string hmc_string_util::vec_to_array_jsonA(std::vector<std::string> item_list)
{
    return vec_to_array_json_any(item_list);
}

std::string hmc_string_util::vec_to_array_json(std::vector<std::string> item_list)
{
    return vec_to_array_json_any(item_list);
}

std::string hmc_string_util::map_to_jsonA(std::any item_list)
{
    std::string result = "{";

    if (!item_list.has_value())
    {
        return "{}";
    }
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, std::string, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, char, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, char *, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, const char *, std::string);

    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int64_t, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long long, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int32_t, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, size_t, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long int, std::string);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, std::string);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, char, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int64_t, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long long, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int32_t, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, size_t, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long int, char);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, char);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, char, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int64_t, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long long, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int32_t, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, size_t, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long int, char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, char *);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int64_t, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long long, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, int32_t, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, size_t, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, long int, const char *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonA, std::map, unsigned long, const char *);

    return "{}";
}

std::wstring hmc_string_util::map_to_jsonW(std::any item_list)
{

    if (!item_list.has_value())
    {
        return L"{}";
    }
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, std::wstring, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, wchar_t, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, wchar_t *, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, const wchar_t *, std::wstring);

    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int64_t, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long long, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int32_t, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, size_t, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long int, std::wstring);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, std::wstring);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, wchar_t, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int64_t, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long long, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int32_t, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, size_t, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long int, wchar_t);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, wchar_t);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, wchar_t, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int64_t, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long long, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int32_t, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, size_t, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long int, wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, wchar_t *);

    // string
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int64_t, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long long, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, int32_t, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, size_t, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, long int, const wchar_t *);
    ___hmc_string_util_diff_any_value_eval_map_to_fn_map(item_list, map_to_jsonW, std::map, unsigned long, const wchar_t *);

    return L"{}";
}

bool hmc_string_util::is_map_json(std::any input)
{

    if (!input.has_value())
    {
        return false;
    }
    if (
        ___hmc_string_util_eqt(typeid(std::map<std::string, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<int, std::string>)) ||
        //___hmc_string_util_eqt(typeid(std::map<int64_t, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long long, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<int32_t, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<size_t, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<unsigned long, std::string>)) ||
        //___hmc_string_util_eqt(typeid(std::map<long, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long int, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, char>)) ||
        //___hmc_string_util_eqt(typeid(std::map<int64_t, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long long, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<int32_t, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<size_t, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<unsigned long, char>)) ||
        //___hmc_string_util_eqt(typeid(std::map<long, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long int, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, char *>)) ||
        //___hmc_string_util_eqt(typeid(std::map<int64_t, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long long, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<int32_t, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<size_t, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<unsigned long, char *>)) ||
        //___hmc_string_util_eqt(typeid(std::map<long, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long int, char *>)) ||
        //___hmc_string_util_eqt(typeid(std::map<int64_t, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long long, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<int32_t, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<size_t, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<unsigned long, const char *>)) ||
        //___hmc_string_util_eqt(typeid(std::map<long, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<long int, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, std::string>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, char>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, const char *>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, int>)) ||
        //___hmc_string_util_eqt(typeid(std::map<std::string, int64_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, long long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, int32_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, size_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, unsigned long>)) ||
        //___hmc_string_util_eqt(typeid(std::map<std::string, long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<std::string, long int>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, char>)) ||
        //___hmc_string_util_eqt(typeid(std::map<char, int64_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, long long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, int32_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, size_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, unsigned long>)) ||
        //___hmc_string_util_eqt(typeid(std::map<char, long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char, long int>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, char>)) ||
        //___hmc_string_util_eqt(typeid(std::map<char *, int64_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, long long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, int32_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, size_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, unsigned long>)) ||
        //___hmc_string_util_eqt(typeid(std::map<char *, long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<char *, long int>)) ||
        //___hmc_string_util_eqt(typeid(std::map<const char *, int64_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, long long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, int32_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, size_t>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, unsigned long>)) ||
        //___hmc_string_util_eqt(typeid(std::map<const char *, long>)) ||
        ___hmc_string_util_eqt(typeid(std::map<const char *, long int>)))
    {
        return true;
    }
    return false;
}

bool hmc_string_util::is_int(std::any input)
{

    if (!input.has_value())
    {
        return false;
    }
    if (
        ___hmc_string_util_eq(std::uint8_t) ||
        ___hmc_string_util_eq(unsigned int) ||
        ___hmc_string_util_eq(unsigned long) ||
        ___hmc_string_util_eq(unsigned long long) ||
        ___hmc_string_util_eq(int) ||
        ___hmc_string_util_eq(long long) ||
        ___hmc_string_util_eq(long))
    {
        return true;
    }
    return false;
}

bool hmc_string_util::is_vec_json(std::any input)
{

    if (!input.has_value())
    {
        return false;
    }
    if (
        ___hmc_string_util_eq(std::vector<char *>) ||
        ___hmc_string_util_eq(std::vector<std::uint8_t>) ||
        ___hmc_string_util_eq(std::vector<unsigned int>) ||
        //___hmc_string_util_eq(std::vector<unsigned long>) ||
        ___hmc_string_util_eq(std::vector<unsigned long>) ||
        ___hmc_string_util_eq(std::vector<unsigned long long>) ||
        ___hmc_string_util_eq(std::vector<int>) ||
        ___hmc_string_util_eq(std::vector<long>) ||
        ___hmc_string_util_eq(std::vector<short>) ||
        ___hmc_string_util_eq(std::vector<long double>) ||
        ___hmc_string_util_eq(std::vector<double>) ||
        ___hmc_string_util_eq(std::vector<std::string>) ||
        ___hmc_string_util_eq(std::vector<std::wstring>) ||
        ___hmc_string_util_eq(std::vector<wchar_t *>) ||
        ___hmc_string_util_eq(std::vector<wchar_t>))
    {
        return true;
    }
    return false;
}

bool hmc_string_util::any_to_string(std::any input, std::string &output)
{
    bool result = false;
#define Input(types) std::any_cast<types>(input)

    output.clear();

    if (input.type() == typeid(char *))
    {
        output.append(Input(char *));
        return true;
    }

    if (input.type() == typeid(std::string))
    {
        output.append(Input(std::string));
        return true;
    }

    if (input.type() == typeid(std::wstring))
    {
        output.append(utf16_to_ansi(Input(std::wstring)));
        return true;
    }

    if (input.type() == typeid(wchar_t *))
    {
        output.append(utf16_to_ansi(Input(wchar_t *)));
        return true;
    }

    if (input.type() == typeid(char))
    {
        output.push_back(std::any_cast<char>(input));
        return true;
    }

    if (input.type() == typeid(wchar_t))
    {
        std::wstring temp = std::wstring();
        temp.push_back(Input(wchar_t));
        output.append(utf16_to_ansi(temp));
        return true;
    }

#define ReturnToArrayA(types)                                                           \
    if (input.type() == typeid(types))                                                  \
    {                                                                                   \
        output.append(hmc_string_util::vec_to_array_json(std::any_cast<types>(input))); \
        return true;                                                                    \
    }

    ReturnToArrayA(std::vector<std::uint8_t>);
    // ReturnToArray(std::vector<unsigned wchar_t>);
    ReturnToArrayA(std::vector<unsigned int>);
    ReturnToArrayA(std::vector<unsigned long>);
    ReturnToArrayA(std::vector<unsigned long>);
    ReturnToArrayA(std::vector<unsigned long long>);

    ReturnToArrayA(std::vector<int>);
    ReturnToArrayA(std::vector<long>);
    ReturnToArrayA(std::vector<short>);
    ReturnToArrayA(std::vector<long double>);
    ReturnToArrayA(std::vector<double>);
    // ReturnToArray(std::vector<char *>);
    ReturnToArrayA(std::vector<std::string>);

    if (input.type() == typeid(std::vector<std::wstring>))
    {
        std::wstring data = hmc_string_util::vec_to_array_json(std::any_cast<std::vector<std::wstring>>(input));
        output.append(utf16_to_ansi(data));
        return true;
    }

    // 整数型
    if (is_int(input))
    {
        output.append(std::to_string(Input(unsigned long long)));
        return true;
    }

    // 浮点
    if (___hmc_string_util_eq(short) || ___hmc_string_util_eq(long double) || ___hmc_string_util_eq(double))
    {
        output.append(std::to_string(Input(long double)));
        return true;
    }

    if (is_map_json(input))
    {
        output.append(map_to_jsonA(input));
        return true;
    }

    return result;
}

bool hmc_string_util::any_to_string(std::any input, std::wstring &output)
{
    bool result = false;
#define Input(types) std::any_cast<types>(input)

    output.clear();

    if (input.type() == typeid(char *))
    {
        output.append(ansi_to_utf16(Input(char *)));
        return true;
    }

    if (input.type() == typeid(std::string))
    {
        output.append(ansi_to_utf16(Input(std::string)));
        return true;
    }

    if (input.type() == typeid(std::wstring))
    {
        output.append(Input(std::wstring));
        return true;
    }

    if (input.type() == typeid(wchar_t *))
    {
        output.append(Input(wchar_t *));
        return true;
    }

    if (input.type() == typeid(char))
    {
        output.push_back(std::any_cast<char>(input));
        return true;
    }

    if (input.type() == typeid(wchar_t))
    {
        std::wstring temp = std::wstring();
        temp.push_back(Input(wchar_t));
        output.append(temp);
        return true;
    }

#define ReturnToArrayW(types)                                                               \
    if (input.type() == typeid(types))                                                      \
    {                                                                                       \
        std::string temp = hmc_string_util::vec_to_array_json(std::any_cast<types>(input)); \
        output.append(ansi_to_utf16(temp));                                                 \
        return true;                                                                        \
    }

    ReturnToArrayW(std::vector<std::uint8_t>);
    // ReturnToArray(std::vector<unsigned wchar_t>);
    ReturnToArrayW(std::vector<unsigned int>);
    ReturnToArrayW(std::vector<unsigned long>);
    ReturnToArrayW(std::vector<unsigned long>);
    ReturnToArrayW(std::vector<unsigned long long>);

    ReturnToArrayW(std::vector<int>);
    ReturnToArrayW(std::vector<long>);
    ReturnToArrayW(std::vector<short>);
    ReturnToArrayW(std::vector<long double>);
    ReturnToArrayW(std::vector<double>);
    // ReturnToArray(std::vector<char *>);
    ReturnToArrayW(std::vector<std::string>);

    if (input.type() == typeid(std::vector<std::wstring>))
    {
        std::wstring data = hmc_string_util::vec_to_array_json(std::any_cast<std::vector<std::wstring>>(input));
        output.append(data);
        return true;
    }

    // 整数型
    if (is_int(input))
    {
        output.append(std::to_wstring(Input(unsigned long long)));
        return true;
    }

    // 浮点
    if (___hmc_string_util_eq(short) || ___hmc_string_util_eq(long double) || ___hmc_string_util_eq(double))
    {
        output.append(std::to_wstring(Input(long double)));
        return true;
    }

    if (is_map_json(input))
    {
        output.append(map_to_jsonW(input));
        return true;
    }

    return result;
}

std::wstring hmc_string_util::push_json_value(std::wstring key, std::any value, bool is_append, bool esp_type)
{
    std::wstring result;
    if (is_append)
    {
        result.append(L",");
    }
    result.append(hmc_json_util::escapeJsonString(key, true));
    result.append(L":");
    std::wstring output;

    if (any_to_string(value, output))
    {
        if (is_int(value))
        {
            result.append(output);
        }
        // else if (is)(value)) {

        //}
        else
        {
            result.append(hmc_json_util::escapeJsonString(output, (esp_type ? true : false)));
        }
    }
    else
    {
        result.append(hmc_json_util::escapeJsonString(L"undefined", false));
    }

    return result;
}

std::string hmc_string_util::vec_to_array_json(const std::vector<char> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<char>>(data_list));
}

std::string hmc_string_util::vec_to_array_json(const std::vector<double> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<double>>(data_list));
}
std::string hmc_string_util::vec_to_array_json(const std::vector<long double> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<long double>>(data_list));
}
std::string hmc_string_util::vec_to_array_json(const std::vector<short> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<short>>(data_list));
}
std::string hmc_string_util::vec_to_array_json(const std::vector<long> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<long>>(data_list));
}
std::string hmc_string_util::vec_to_array_json(const std::vector<int> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<int>>(data_list));
}
std::string hmc_string_util::vec_to_array_json(const std::vector<unsigned long long> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<unsigned long long>>(data_list));
}

std::string hmc_string_util::vec_to_array_json(const std::vector<long long> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<long long>>(data_list));
}

std::string hmc_string_util::vec_to_array_json(const std::vector<unsigned int> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<unsigned int>>(data_list));
}

std::string hmc_string_util::vec_to_array_json(const std::vector<std::uint8_t> data_list)
{
    return vec_to_array_json_any(std::any_cast<std::vector<std::uint8_t>>(data_list));
}

std::string hmc_string_util::vec_to_array_json(const std::vector<unsigned long> data_list)
{

    return vec_to_array_json_any(std::any_cast<std::vector<unsigned long>>(data_list));
}

#endif //_HAS_CXX17

std::wstring hmc_string_util::unicodeStringToWString(HMC_UNICODE_STRING unicodeString)
{
    std::wstring result;
    if (unicodeString.Buffer)
    {
        result = std::wstring(unicodeString.Buffer, unicodeString.Length / sizeof(wchar_t));
    }
    return result;
}

std::string hmc_string_util::base64_encode(const std::string &input)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded_temp;

    if (input.empty())
    {
        return encoded_temp;
    }

    // 在Base64编码中，每3个字节的数据会编码为4个字符
    size_t base64_size = ((input.size() / 3) + 1) * 4 + 52;
    encoded_temp.reserve(base64_size);

    int i = 0;
    int j = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    sizeof(unsigned char) == sizeof(char);

    for (char c : input)
    {
        array3[i++] = c;
        if (i == 3)
        {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;

            for (int k = 0; k < 4; k++)
            {
                auto it = base64_chars[array4[k]];
                encoded_temp.push_back(it);
            }

            i = 0;
        }
    }

    if (i != 0)
    {
        for (int k = i; k < 3; k++)
            array3[k] = '\0';

        // 对照表
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        array4[3] = array3[2] & 0x3f;

        for (int k = 0; k < i + 1; k++)
        {
            auto it = base64_chars[array4[k]];
            encoded_temp.push_back(it);
        }

        //  尾部不足4的倍数 添加=
        while (i++ < 3)
        {
            encoded_temp.push_back('=');
        }
    }

    return std::string(encoded_temp.begin(), encoded_temp.end());
}

std::string hmc_string_util::base64_encode(const std::vector<std::uint8_t> &input)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;

    if (input.empty())
    {
        return output;
    }

    // 在Base64编码中，每3个字节的数据会编码为4个字符
    size_t base64_size = ((input.size() / 3) + 1) * 4 + 52;
    output.reserve(base64_size);

    const unsigned char *bytes_to_encode = &input[0];
    size_t len = input.size();
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
            {
                const auto it = base64_chars[char_array_4[i]];
                output.push_back(it);
            }
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
        {
            const auto it = base64_chars[char_array_4[i]];
            output.push_back(it);
        }

        while ((i++ < 3))
        {
            output.push_back('=');
        }
    }

    return std::string(output.begin(), output.end());
}

#ifndef HMC_IMPORT_HMC_BYTE_UTIL_H
namespace hmc_string_util
{
    std::vector<std::uint8_t> int32ToBytes(const std::uint32_t data)
    {
        std::vector<std::uint8_t> bytes = std::vector<std::uint8_t>();
        bytes.reserve(4);
        bytes.push_back((data >> 24) & 0xFF);
        bytes.push_back((data >> 16) & 0xFF);
        bytes.push_back((data >> 8) & 0xFF);
        bytes.push_back(data & 0xFF);
        return bytes;
    }

    std::vector<std::uint8_t> wstringToBytes(const wchar_t &bytes_string)
    {
        std::vector<std::uint8_t> buff = {};

        // utf32
        if (bytes_string >= 0x10FFFF)
        {
            std::uint32_t value = static_cast<std::uint32_t>(bytes_string);
            auto int32Bytes = int32ToBytes(value);

            for (size_t i = 0; i < int32Bytes.size(); i++)
            {
                buff.push_back(int32Bytes.at(i));
            }
        }
        else
        {

            // utf16
            std::uint16_t value = static_cast<std::uint16_t>(bytes_string);
            std::uint8_t partA = (std::uint8_t)((value & 0xFF00) >> 8);
            std::uint8_t partB = (std::uint8_t)(value & 0x00FF);
            buff.push_back(partA);
            buff.push_back(partB);
        }

        return buff;
    }
}

#endif // HMC_IMPORT_HMC_BYTE_UTIL_H

std::string hmc_string_util::base64_encode(const std::wstring &input)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded_temp;

    if (input.empty())
    {
        return encoded_temp;
    }

    return std::string(encoded_temp.begin(), encoded_temp.end());
}

bool hmc_string_util::includes(const std::wstring &text, const std::wstring &substring)
{
    return text.find(substring) != text.npos;
}

bool hmc_string_util::includes(const std::string &text, const std::string &substring)
{
    return text.find(substring) != text.npos;
}

bool hmc_string_util::includes(const std::string &text, const char &substring)
{
    return text.find(substring) != text.npos;
}

bool hmc_string_util::includes(const std::wstring &text, const wchar_t &substring)
{
    return text.find(substring) != text.npos;
}

inline std::string hmc_string_util::slice(const std::string &str, int start, int end)
{
    int length = static_cast<int>(str.length());

    if (start < 0)
    {
        start = length + start;
    }
    if (end < 0)
    {
        end = length + end + 1;
    }

    int i_temp = (start < length) ? start : length;
    start = (0 > i_temp) ? 0 : i_temp;

    int a_temp = (end > length) ? end : length;
    end = (start > a_temp) ? start : a_temp;

    return str.substr(start, static_cast<long long>(end) - start);
}

inline std::wstring hmc_string_util::slice(const std::wstring &str, int start, int end)
{
    int length = static_cast<int>(str.length());

    if (start < 0)
    {
        start = length + start;
    }
    if (end < 0)
    {
        end = length + end + 1;
    }

    int i_temp = (start < length) ? start : length;
    start = (0 > i_temp) ? 0 : i_temp;

    int a_temp = (end > length) ? end : length;
    end = (start > a_temp) ? start : a_temp;

    return str.substr(start, static_cast<long long>(end) - start);
}

long long hmc_string_util::search(const std::string &str, const std::string &pattern)
{
    std::regex regexPattern(pattern);
    std::smatch match;
    if (std::regex_search(str, match, regexPattern))
    {
        return match.position();
    }
    return -1;
}

std::vector<std::string> hmc_string_util::match(const std::string &str, const std::string &pattern)
{
    std::regex regexPattern(pattern);
    std::smatch match;
    std::vector<std::string> matches;

    std::string::const_iterator searchStart(str.cbegin());
    while (std::regex_search(searchStart, str.cend(), match, regexPattern))
    {
        matches.push_back(match.str());
        searchStart = match.suffix().first; // Move to next match
    }

    return matches;
}

long long hmc_string_util::indexOf(const std::string &str, const std::string &substr)
{
    size_t pos = str.find(substr);
    if (pos != str.npos)
    {
        long long pos_len = static_cast<long long>(pos);
        if (pos_len < static_cast<long long>(str.size()))
            return pos_len;
    }
    return -1;
}

std::vector<size_t> hmc_string_util::indexOfs(const std::string &str, const std::string &substr)
{
    std::vector<size_t> positionList = {};

    size_t pos = str.find(substr);
    if (pos != str.npos)
    {
        do
        {
            positionList.push_back(pos);
            if (pos != str.npos)
                pos = str.find(substr, pos + 1);
        } while (pos != str.npos);
    }
    return positionList;
}

long long hmc_string_util::indexOf(const std::string &str, const char &substr)
{
    long long pos = str.find(substr);
    if (pos != str.npos)
    {
        long long pos_len = static_cast<long long>(pos);
        if (pos_len < static_cast<long long>(str.size()))
            return pos_len;
    }
    return -1;
}

std::vector<size_t> hmc_string_util::indexOfs(const std::string &str, const char &substr)
{
    std::vector<size_t> positionList = {};
    for (size_t i = 0; i < str.size(); i++)
    {
        auto &at = str.at(i);
        if (at == substr)
        {
            positionList.push_back(i);
        }
    }
    return positionList;
}

long long hmc_string_util::search(const std::wstring &str, const std::wstring &pattern)
{
    std::wregex regexPattern(pattern);
    std::wsmatch match;
    if (std::regex_search(str, match, regexPattern))
    {
        return match.position();
    }
    return -1;
}

std::vector<std::wstring> hmc_string_util::match(const std::wstring &str, const std::wstring &pattern)
{
    std::wregex regexPattern(pattern);
    std::wsmatch match;
    std::vector<std::wstring> matches;

    std::wstring::const_iterator searchStart(str.cbegin());
    while (std::regex_search(searchStart, str.cend(), match, regexPattern))
    {
        matches.push_back(match.str());
        searchStart = match.suffix().first; // Move to next match
    }

    return matches;
}

long long hmc_string_util::indexOf(const std::wstring &str, const std::wstring &substr)
{
    size_t pos = str.find(substr);
    if (pos != str.npos)
    {
        return static_cast<long long>(pos);
    }
    return -1;
}

std::vector<size_t> hmc_string_util::indexOfs(const std::wstring &str, const std::wstring &substr)
{
    std::vector<size_t> positionList = {};

    size_t pos = str.find(substr);
    if (pos != str.npos)
    {
        do
        {
            positionList.push_back(pos);
            if (pos != str.npos)
            {
                pos = str.find(substr, pos + 1);
            }
            if (pos >= str.size())
                break;
        } while (pos != str.npos);
    }
    return positionList;
}

long long hmc_string_util::indexOf(const std::wstring &str, const wchar_t &substr)
{
    long long pos = str.find(substr);
    if (pos != str.npos)
    {
        return static_cast<long long>(pos);
    }
    return -1;
}

std::vector<size_t> hmc_string_util::indexOfs(const std::wstring &str, const wchar_t &substr)
{
    std::vector<size_t> positionList = {};
    for (size_t i = 0; i < str.size(); i++)
    {
        auto &at = str.at(i);
        if (at == substr)
        {
            positionList.push_back(i);
        }
    }
    return positionList;
}

std::string hmc_string_util::replaceRegex(const std::string &input, const std::string &pattern, const std::string &replacement, std::regex_constants::syntax_option_type flags)
{
    std::regex re(pattern, flags);
    std::string output = std::regex_replace(input, re, replacement);
    return output;
}

std::string hmc_string_util::replaceRegex(const std::string &input, const std::string &pattern, const std::string &replacement)
{
    std::regex re(pattern);
    std::string output = std::regex_replace(input, re, replacement);
    return output;
}

std::wstring hmc_string_util::replaceRegex(const std::wstring &input, const std::wstring &pattern, const std::wstring &replacement, std::regex_constants::syntax_option_type flags)
{
    std::wregex re(pattern, flags);
    std::wstring output = std::regex_replace(input, re, replacement);
    return output;
}

std::wstring hmc_string_util::replaceRegex(const std::wstring &input, const std::wstring &pattern, const std::wstring &replacement)
{
    std::wregex re(pattern);
    std::wstring output = std::regex_replace(input, re, replacement);
    return output;
}

inline size_t hmc_string_util::removeAllCharPtr(std::wstring &input, wchar_t matching)
{
    size_t replacementFrequency = 0;

    auto end_pos = input.size();
    while ((end_pos = input.find_last_not_of(matching)) != input.size() - 1)
    {
        input.erase(end_pos + 1);
        replacementFrequency++;
    }

    return replacementFrequency;
}

inline bool hmc_string_util::is_letter(char Input)
{
    return (Input >= L'A' && Input <= L'Z') || (Input >= L'a' && Input <= L'z');
}

inline bool hmc_string_util::is_letter(wchar_t Input)
{
    return (Input >= L'A' && Input <= L'Z') || (Input >= L'a' && Input <= L'z');
}

inline size_t hmc_string_util::removeAllCharPtr(std::string &input, char matching)
{
    size_t replacementFrequency = 0;

    auto end_pos = input.size();
    while ((end_pos = input.find_last_not_of(matching)) != input.size() - 1)
    {
        input.erase(end_pos + 1);
        replacementFrequency++;
    }

    return replacementFrequency;
}

std::string hmc_string_util::aou8_to_utf8(const std::string input)
{
    std::string result;
    if (
        hmc_string_util::is_ascii(static_cast<const std::string>(input)) ||
        hmc_string_util::is_utf8(static_cast<const std::string>(input)))
    {
        result.append(input.begin(), input.end());
        return result;
    }
    result = std::move(hmc_string_util::ansi_to_utf8(input));
    return result;
}

std::wstring hmc_string_util::aou8_to_utf16(const std::string input)
{
    std::wstring result;

    if (hmc_string_util::is_ascii(static_cast<const std::string>(input)))
    {
        result = std::move(hmc_string_util::asciiConvers(input));
        return result;
    }

    if (hmc_string_util::is_utf8(static_cast<const std::string>(input)))
    {
        result = std::move(hmc_string_util::utf8_to_utf16(input));
        return result;
    }

    result = std::move(hmc_string_util::ansi_to_utf16(input));

    return result;
}
