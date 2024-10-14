#include "hmc_json_util.h"
#include "hmc_string_util.h"

namespace hmc_json_util
{
    namespace hmc_define_util
    {
#define hmc_define_to_Json_Value_caseA(caseIndex) \
    case caseIndex:                               \
        return std::to_string(std::get<caseIndex>(value));
#define hmc_define_to_Json_Value_caseW(caseIndex) \
    case caseIndex:                               \
        return std::to_wstring(std::get<caseIndex>(value));
    }

}

namespace HMC_TEMPLATE_FUNC
{

    template <typename T, typename char_type>
    static T formatJson(T json, char_type indent, size_t indentSize)
    {
        T result;

        size_t level = 0;

        for (size_t index = 0; index < json.size(); index++)
        {
            auto &at = json.at(index);

            if (level != 0 && json.back() == '\n')
            {
                result.append(level * indentSize, indent);
            }

            switch (at)
            {
            case '{':
            case '[':
                result.push_back(at);
                result.push_back('\n');
                level++;
                result.append(level * indentSize, indent);
                break;
            case ',':
                result.push_back(at);
                result.push_back('\n');
                result.append(level * indentSize, indent);
                break;
            case '}':
            case ']':
                result.push_back('\n');

                if (level >= 1)
                {
                    level--;
                }

                result.append(level * indentSize, indent);
                result.push_back(at);
                break;
            default:
                result.push_back(at);
                break;
            }
        }

        return result;
    }

    template <typename T>
    static inline T escapeJsonString(const T &input, bool is_to_value)
    {
        T output;

        if (is_to_value)
        {
            output.push_back('"');
        }
        for (size_t i = 0; i < input.size(); i++)
        {
            auto &ch = input.at(i);

            switch (ch)
            {
            case '\0':
                break;
            case '"':
                output.push_back('\\');
                output.push_back('\"');
                break;
            case '\\':
                output.push_back('\\');
                output.push_back('\\');
                break;
            case '\b':
                output.push_back('\\');
                output.push_back('b');
                break;
            case '\f':
                output.push_back('\\');
                output.push_back('f');
                break;
            case '\n':
                output.push_back('\\');
                output.push_back('n');
                break;
            case '\r':
                output.push_back('\\');
                output.push_back('r');
                break;
            case '\t':
                output.push_back('\\');
                output.push_back('t');
                break;
            default:
                output.push_back(ch);
                break;
            }
        }
        if (is_to_value)
        {
            output.push_back('"');
        }
        return output;
    }

    template <typename T, typename ResultT>
    static inline ResultT escapeJsonString(const T &input)
    {
        ResultT output;

        static_assert(
            std::is_same_v<T, bool> ||
                std::is_integral_v<T> ||
                std::is_floating_point_v<T>,
            "Unsupported type preset escape (不支持的类型预设转义)");

        if constexpr (std::is_same_v<T, bool>)
        {

            if (input)
            {
                output = ResultT({'t', 'r', 'u', 'e'});
            }
            else
            {
                output = ResultT({'f', 'a', 'l', 's', 'e'});
            }
            return output;
        }

        if constexpr (std::is_integral_v<T>(input) || std::is_pointer_v<T>(input))
        {

            if constexpr (std::is_same_v<ResultT, std::wstring>)
            {
                output = std::to_wstring(input);
            }

            if constexpr (std::is_same_v<ResultT, std::string>)
            {
                output = std::to_string(input);
            }
        }

        output = ResultT({'n', 'u', 'l', 'l'});
        return output;
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

    template <typename T>
    static T unEscapeJsonString(const T &sourcePtr)
    {
        using char_type = typename T::value_type;
        using size_type = typename T::size_type;

        T ouput = T();

        if (sourcePtr.empty())
        {
            return ouput;
        }

        ouput.reserve(sourcePtr.size() + 50);

        T hex_str;
        hex_str.reserve(50);

        size_type ignore_size = 1;
        unsigned int hex;

        for (size_type i = 0; i < sourcePtr.size(); i++)
        {
            char_type at = sourcePtr.at(i);

            if ((at == '\\') && (i + 1 < sourcePtr.size()))
            {
                ignore_size = 1;

                char_type at_next = sourcePtr[i + 1];

                switch (at_next)
                {
                case '"':
                {
                    ouput.push_back('\"');
                    break;
                }
                case '\\':
                {
                    ouput.push_back('\\');
                    break;
                }
                case '/':
                {
                    ouput.push_back('/');
                    break;
                }
                case 't':
                {
                    ouput.push_back('\t');
                    break;
                }
                case 'n':
                {
                    ouput.push_back('\n');
                    break;
                }
                case 'r':
                {
                    ouput.push_back('\r');
                    break;
                }
                case 'b':
                {
                    ouput.push_back('\b');
                    break;
                }
                case 'f':
                {
                    ouput.push_back('\f');
                    break;
                }
                case 'u':
                {

                    ignore_size = 5;

                    if ((i + 4) < sourcePtr.size())
                    {

                        hex_str = sourcePtr.substr(i + 4, 2);

                        if (hex_str.empty())
                            continue;

                        hex = static_cast<unsigned int>(std::stoi(hex_str, nullptr, 16));

                        // hex = static_cast<unsigned int>(std::strtoul(hex_str.c_str(), nullptr, 16));
                        ouput.push_back(static_cast<char_type>(hex));
                        hex = 0;
                        ignore_size = 1;
                        hex_str.clear();
                    }

                    break;
                }

                default:
                    break;
                }

                i += ignore_size;
                continue;
            }

            ouput.push_back(at);
        }

        return ouput;
    }

}

namespace hmc_json_util
{

    namespace HMC = HMC_TEMPLATE_FUNC;

    std::wstring hmc_json_util::formatJson(std::wstring json, wchar_t indent, size_t indentSize) { return HMC::formatJson<std::wstring, wchar_t>(json, indent, indentSize); }
    std::string hmc_json_util::formatJson(std::string json, char indent, size_t indentSize) { return HMC::formatJson<std::string, char>(json, indent, indentSize); }

    inline std::string hmc_json_util::escapeJsonString(const std::string &input, bool is_to_value) { return HMC::escapeJsonString<std::string>(input, is_to_value); }
    inline std::wstring hmc_json_util::escapeJsonString(const std::wstring &input, bool is_to_value) { return HMC::escapeJsonString<std::wstring>(input, is_to_value); }

    std::string hmc_json_util::unEscapeJsonString(const std::string &source) { return HMC::unEscapeJsonString<std::string>(source); }
    std::wstring hmc_json_util::unEscapeJsonString(const std::wstring &source) { return HMC::unEscapeJsonString<std::wstring>(source); }

}

std::string hmc_json_util::toJsonValueA(hmc_json_util::NumberType value)
{
    switch (value.index())
    {
        hmc_define_to_Json_Value_caseA(0);
        hmc_define_to_Json_Value_caseA(1);
        hmc_define_to_Json_Value_caseA(2);
        hmc_define_to_Json_Value_caseA(3);
        hmc_define_to_Json_Value_caseA(4);
        hmc_define_to_Json_Value_caseA(5);
        hmc_define_to_Json_Value_caseA(6);
        hmc_define_to_Json_Value_caseA(7);
        hmc_define_to_Json_Value_caseA(8);
        hmc_define_to_Json_Value_caseA(9);
        hmc_define_to_Json_Value_caseA(10);
        hmc_define_to_Json_Value_caseA(11);
        hmc_define_to_Json_Value_caseA(12);
        hmc_define_to_Json_Value_caseA(13);
        hmc_define_to_Json_Value_caseA(14);
        hmc_define_to_Json_Value_caseA(15);
        hmc_define_to_Json_Value_caseA(16);
        hmc_define_to_Json_Value_caseA(17);
        // hmc_define_to_Json_Value_caseA(18);
        // hmc_define_to_Json_Value_caseA(19);
        // hmc_define_to_Json_Value_caseA(20);
        // hmc_define_to_Json_Value_caseA(21);
        // hmc_define_to_Json_Value_caseA(22);
        // hmc_define_to_Json_Value_caseA(23);
        // hmc_define_to_Json_Value_caseA(24);
        // hmc_define_to_Json_Value_caseA(25);
    }
    return "0";
}

std::wstring hmc_json_util::toJsonValueW(hmc_json_util::NumberType value)
{
    switch (value.index())
    {
        hmc_define_to_Json_Value_caseW(0);
        hmc_define_to_Json_Value_caseW(1);
        hmc_define_to_Json_Value_caseW(2);
        hmc_define_to_Json_Value_caseW(3);
        hmc_define_to_Json_Value_caseW(4);
        hmc_define_to_Json_Value_caseW(5);
        hmc_define_to_Json_Value_caseW(6);
        hmc_define_to_Json_Value_caseW(7);
        hmc_define_to_Json_Value_caseW(8);
        hmc_define_to_Json_Value_caseW(9);
        hmc_define_to_Json_Value_caseW(10);
        hmc_define_to_Json_Value_caseW(11);
        hmc_define_to_Json_Value_caseW(12);
        hmc_define_to_Json_Value_caseW(13);
        hmc_define_to_Json_Value_caseW(14);
        hmc_define_to_Json_Value_caseW(15);
        hmc_define_to_Json_Value_caseW(16);
        hmc_define_to_Json_Value_caseW(17);
        // hmc_define_to_Json_Value_caseW(18);
        // hmc_define_to_Json_Value_caseW(19);
        // hmc_define_to_Json_Value_caseW(20);
        // hmc_define_to_Json_Value_caseW(21);
        // hmc_define_to_Json_Value_caseW(22);
        // hmc_define_to_Json_Value_caseW(23);
        // hmc_define_to_Json_Value_caseW(24);
        // hmc_define_to_Json_Value_caseW(25);
    }
    return L"0";
}
