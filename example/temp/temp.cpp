#include <iostream>
#include <any>
#include <functional>

namespace HMC_TEMPLATE_FUNC
{

	// using T = std::wstring;
	// using FILTER_T = std::function<bool(wchar_t at, size_t index, T *input)>;

	size_t removeMatchCharPtr(std::wstring &input, std::function<bool(wchar_t at, size_t index, std::wstring *input)> func);

	template <typename T, typename T2>
	static bool VecContainChar(T _Vec, T2 _Char)
	{
		bool result = false;

		for (size_t i = 0; i < _Vec.size(); i++)
		{
			auto &at = _Vec.at(i);
			if (at == _Char)
			{
				return true;
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
				std::is_same_v<FILTER_T, std::function<bool(char at, size_t index, T* input)>> ||
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

}

namespace hmc_string_util
{
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::wstring *StrInput, wchar_t filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::wstring *StrInput, std::vector<wchar_t> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index, std::wstring *input)> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index)> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at)> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::string *StrInput, char filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::string *StrInput, std::vector<char> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index, std::string *input)> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index)> filter);
	// 删除匹配条件的字符
	extern size_t removeMatchChar(std::string *StrInput, std::function<bool(char at)> filter);

}

size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, wchar_t filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::wstring, wchar_t>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::vector<wchar_t> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::wstring, std::vector<wchar_t>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index, std::wstring *input)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::wstring, std::function<bool(wchar_t at, size_t index, std::wstring * input)>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at, size_t index)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::wstring, std::function<bool(wchar_t at, size_t index)>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::wstring *StrInput, std::function<bool(wchar_t at)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::wstring, std::function<bool(wchar_t at)>>(StrInput, filter); }

size_t hmc_string_util::removeMatchChar(std::string *StrInput, char filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::string, char>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::vector<char> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::string, std::vector<char>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index, std::string *input)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::string, std::function<bool(char at, size_t index, std::string *input)>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at, size_t index)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::string, std::function<bool(char at, size_t index)>>(StrInput, filter); }
size_t hmc_string_util::removeMatchChar(std::string *StrInput, std::function<bool(char at)> filter) { return HMC_TEMPLATE_FUNC::removeMatchChar<std::string, std::function<bool(char at)>>(StrInput, filter); }

int main()
{
	using String = std::wstring;

	auto str = String({'s', 's', 'a', 't', 'a', '0', '1', '2', 't', 'T', 'Y', '3', '4', '5', '6', '7', 'Y'});

	return 0;
}
