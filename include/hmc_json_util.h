#pragma once

// 防止重复导入
#ifndef MODE_INTERNAL_INCLUDE_HMC_TEXT_JSON_UTIL_H
#define MODE_INTERNAL_INCLUDE_HMC_TEXT_JSON_UTIL_H

#define EPOCH_TIME UINT64_C(116444736000000000) // 1970.01.01 00:00:000 in MS Filetime

#include <string>

namespace hmc_json_util
{

    /**
     * @brief 反特殊字符序列化
     *
     * @param sourcePtr
     */
    extern std::string unEscapeJsonString(const std::string &source);
    /**
     * @brief 反特殊字符序列化
     *
     * @param sourcePtr
     */
    extern std::wstring unEscapeJsonString(const std::wstring &source);

    /**
     * @brief 格式化json字符串缩进
     *
     * @param json
     * @param indent 缩进符号
     * @param indentSize 缩进数量
     * @return std::wstring
     */
    extern std::wstring formatJson(std::wstring json, wchar_t indent = ' ', size_t indentSize = 4);
    /**
     * @brief 格式化json字符串缩进
     *
     * @param json
     * @param indent 缩进符号
     * @param indentSize 缩进数量
     * @return std::wstring
     */
    extern std::string formatJson(std::string json, char indent = ' ', size_t indentSize = 4);

    /**
     * @brief 转义json文本
     *
     * @param input 文本
     * @param is_to_value 添加 "
     * @return std::wstring
     */
    extern std::wstring escapeJsonString(const std::wstring &input, bool is_to_value = false);
    /**
     * @brief 转义json文本
     *
     * @param input 文本
     * @param is_to_value 添加 "
     * @return std::wstring
     */
    extern std::string escapeJsonString(const std::string &input, bool is_to_value = false);

}

#if _HAS_CXX17
#include <variant>
namespace hmc_json_util
{
    typedef std::variant<char,
                         wchar_t,
                         char16_t,
                         char32_t,
                         int,
                         long long,
                         long int,
                         long,
                         long long int,
                         unsigned short,
                         unsigned char,
                         unsigned int,
                         unsigned long,
                         unsigned long long,
                         unsigned long long int,
                         short,
                         long double,
                         double,
                         float>
        NumberType;

    /**
     * @brief 转义到jsonValue
     *
     * @param value
     * @return std::string
     */
    extern std::string toJsonValueA(NumberType value);

    /**
     * @brief 转义到jsonValue
     *
     * @param value
     * @return std::string
     */
    extern std::wstring toJsonValueW(NumberType value);
}
#endif // _HAS_CXX17

#endif // MODE_INTERNAL_INCLUDE_HMC_TEXT_JSON_UTIL_H