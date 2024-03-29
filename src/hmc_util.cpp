﻿#include "./hmc_util.h"

inline size_t hmc_util::removeAllCharPtr(std::wstring &input, wchar_t matching)
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

inline size_t hmc_util::removeAllCharPtr(std::string &input, char matching)
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

inline void hmc_util::replaceAll(std::string &sourcePtr, const std::string from, const std::string to)
{
    size_t start_pos = 0;
    while ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        sourcePtr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline void hmc_util::replaceAll(std::wstring &sourcePtr, const std::wstring from, const std::wstring to)
{
    size_t start_pos = 0;
    while ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        sourcePtr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline void hmc_util::replace(std::string &sourcePtr, const std::string from, const std::string to)
{
    size_t start_pos = 0;
    if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        sourcePtr.replace(start_pos, from.length(), to);
        return;
    }
}

inline void hmc_util::replace(std::wstring &sourcePtr, const std::wstring from, const std::wstring to)
{
    size_t start_pos = 0;
    if ((start_pos = sourcePtr.find(from, start_pos)) != std::string::npos)
    {
        sourcePtr.replace(start_pos, from.length(), to);
        return;
    }
}

inline std::string hmc_util::escapeJsonString(const std::string &input, bool is_to_value)
{
    std::string output;
    if (is_to_value)
    {
        output.push_back('"');
    }

    for (char ch : input)
    {
        switch (ch)
        {

        case '\0':
            break;
        case '"':
            output.append("\\\"");
            break;
        case '\\':
            output.append("\\\\");
            break;
        case '\b':
            output.append("\\b");
            break;
        case '\f':
            output.append("\\f");
            break;
        case '\n':
            output.append("\\n");
            break;
        case '\r':
            output.append("\\r");
            break;
        case '\t':
            output.append("\\t");
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

inline std::wstring hmc_util::escapeJsonString(const std::wstring &input, bool is_to_value)
{
    std::wstring output;

    if (is_to_value)
    {
        output.push_back(L'"');
    }

    for (wchar_t ch : input)
    {
        switch (ch)
        {
        case L'\0':
            break;
        case L'"':
            output.append(L"\\\"");
            break;
        case L'\\':
            output.append(L"\\\\");
            break;
        case L'\b':
            output.append(L"\\b");
            break;
        case L'\f':
            output.append(L"\\f");
            break;
        case L'\n':
            output.append(L"\\n");
            break;
        case L'\r':
            output.append(L"\\r");
            break;
        case L'\t':
            output.append(L"\\t");
            break;
        default:
            output.push_back(ch);
            break;
        }
    }
    if (is_to_value)
    {
        output.push_back(L'"');
    }
    return output;
}

inline std::wstring hmc_util::unicodeStringToWString(UNICODE_STRING unicodeString)
{
    std::wstring result;
    if (unicodeString.Buffer)
    {
        result = std::wstring(unicodeString.Buffer, unicodeString.Length / sizeof(wchar_t));
    }
    return result;
}

inline std::wstring hmc_util::asciiConvers(std::string input)
{
    std::wstring result = std::wstring();

    result.reserve(input.size());

    for (auto &&item : input)
    {
        if (127 >= item && item >= 0)
        {
            result.push_back(static_cast<wchar_t>(item));
        }
    }

    return result;
}

inline std::string hmc_util::asciiConvers(std::wstring input)
{
    std::string result = std::string();
    result.reserve(input.size());

    for (auto &&item : input)
    {
        if (127 >= item && item >= 0)
        {
            result.push_back(static_cast<char>(item));
        }
    }

    return result;
}

inline std::wstring hmc_util::vec2ar(std::vector<std::wstring> input)
{
    std::wstring result = std::wstring();

    DWORD buff_size = MAX_PATH;

    size_t count = input.size();
    for (size_t i = 0; i < count; i++)
    {
        auto it = &input[i];
        buff_size += it->size();
    }

    result.reserve(buff_size);

    result.push_back(L'[');

    count = input.size();
    for (size_t i = 0; i < count; i++)
    {
        auto it = &input[i];
        result.append(it->begin(), it->end());
        if(i+1 < count){
            result.push_back(L',');
        }
    }

    result.push_back(L']');
    return result;
}

inline std::string hmc_util::vec2ar(std::vector<std::string> input)
{
    std::string result = std::string();

    DWORD buff_size = MAX_PATH;

    size_t count = input.size();
    for (size_t i = 0; i < count; i++)
    {
        auto it = &input[i];
        buff_size += it->size();
    }

    result.reserve(buff_size);

    result.push_back('[');

    count = input.size();
    for (size_t i = 0; i < count; i++)
    {
        auto it = &input[i];
        result.append(it->begin(), it->end());
         if(i+1 < count){
            result.push_back(',');
        }
    }

    result.push_back(']');

    return result;
}
