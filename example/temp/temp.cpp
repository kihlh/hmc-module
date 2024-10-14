#include <fstream>
#include <string>
#include <windows.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <io.h>
#include <fcntl.h>


using json = nlohmann::json;

namespace hmc_string_util {

    inline std::string utf16_to_utf8(const std::wstring input)
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
            result.push_back('\0');
        }

        return result;
    }

}
int main() {
    const std::wstring _UTF16_TEXT = L"utf16宽字符車B1234 こんにちは";
    const std::string _GB2312_TEXT = "utf16宽字符車B1234 こんにちは";
    const std::vector<std::uint8_t> _UTF8_TEXT_T = {117,116,102,49,54,229,174,189,229,173,151,231,172,166,232,187,138,66,49,50,51,52,32,227,129,147,227,130,147,227,129,171,227,129,161,227,129,175 };
    const std::string _UTF8_TEXT = std::string(_UTF8_TEXT_T.begin(), _UTF8_TEXT_T.end());


    std::string utf16_to_utf8 = hmc_string_util::utf16_to_utf8(_UTF16_TEXT);
   
    json j;

    j["utf8"] = _UTF8_TEXT;

    j["utf16_to_utf8"] = utf16_to_utf8;


    //set  CONSOLE utf8 log
    int original_mode = _setmode(_fileno(stdin), _O_U8TEXT);

    std::cout<<"  ->["<< original_mode <<"]" << j;

    return 0;
}