#include "hmc_registr_util.h"
#include "hmc_string_util.h"
#include "hmc_util.h"
#include "hmc_json_util.h"

#include <set>
#include <map>
#include <type_traits>
// #include <ShlObj.h>
#include <memory>
#include <array>

#include "hmc_byte_util.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

time_t fileTimeToTimeStamp(FILETIME ftLastWriteTime)
{
	time_t result = 0;
	try
	{
		// 处理时间戳
		const ULONGLONG epochOffset = 116444736000000000ULL;
		ULARGE_INTEGER uli;
		uli.LowPart = ftLastWriteTime.dwLowDateTime;
		uli.HighPart = ftLastWriteTime.dwHighDateTime;
		ULONGLONG timestamp = (uli.QuadPart - epochOffset) / 10000ULL;

		result = static_cast<time_t>(timestamp);
	}
	catch (...)
	{
	}

	return result;
}

bool hmc_registr_util::eq_target_type(DWORD theType, DWORD targetType)
{
	switch (theType)
	{
		// 文本
	case REG_LINK:
	case REG_SZ:
	case REG_MULTI_SZ:
	case REG_EXPAND_SZ:
	{
		return targetType == REG_NONE || targetType == REG_LINK || targetType == REG_SZ || targetType == REG_MULTI_SZ || targetType == REG_EXPAND_SZ;
	}
	case REG_DWORD_BIG_ENDIAN:
	case REG_QWORD:
	case REG_DWORD:
	{
		return targetType == REG_NONE || targetType == REG_DWORD_BIG_ENDIAN || targetType == REG_QWORD || targetType == REG_DWORD;
	}
	case REG_BINARY:
	case REG_RESOURCE_LIST:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	{
		return targetType == REG_NONE || targetType == REG_BINARY || targetType == REG_RESOURCE_LIST || targetType == REG_RESOURCE_REQUIREMENTS_LIST;
	}

	default:
		return false;
	}
}

std::string hmc_registr_util::chType::type_nameA()
{
	return hmc_registr_util::type_nameA(type);
}

std::wstring hmc_registr_util::chType::type_nameW()
{
	return hmc_registr_util::type_nameW(type);
}

bool hmc_registr_util::chType::type_name(std::string &output)
{
	output.clear();
	output.append(type_nameA());
	return !output.empty();
}

bool hmc_registr_util::chType::type_name(std::wstring &output)
{
	output.clear();
	output.append(type_nameW());
	return !output.empty();
}

bool hmc_registr_util::chType::isInt64()
{
	return type == REG_QWORD;
}

bool hmc_registr_util::chType::isString()
{
	return type == REG_SZ || type == REG_MULTI_SZ || type == REG_EXPAND_SZ;
}

bool hmc_registr_util::chType::isNumber()
{
	return type == REG_QWORD || type == REG_DWORD || type == REG_DWORD_BIG_ENDIAN || type == REG_DWORD_LITTLE_ENDIAN || type == REG_QWORD_LITTLE_ENDIAN;
}

bool hmc_registr_util::chType::isInt32()
{
	return type == REG_DWORD || type == REG_DWORD_BIG_ENDIAN || type == REG_DWORD_LITTLE_ENDIAN;
}

bool hmc_registr_util::chType::isBuffer()
{
	return type == REG_BINARY || REG_NONE;
}

bool hmc_registr_util::chType::diffType(DWORD type2)
{
	return type == type2;
}

std::uint32_t hmc_registr_util::chValue::getInt32()
{
	std::uint32_t res = 0ull;
	if (value.empty())
	{
		return res;
	}

	return hmc_byte_util::bytesToInt32(value);

	return res;
}

std::uint16_t hmc_registr_util::chValue::getInt16()
{
	std::uint16_t res = 0ull;
	if (value.empty())
	{
		return res;
	}

	return hmc_byte_util::bytesToInt16(value);

	return res;
}

std::uint64_t hmc_registr_util::chValue::getInt64()
{
	std::uint64_t res = 0ull;
	if (value.empty())
	{
		return res;
	}

	return hmc_byte_util::bytesToInt64(value);
}

std::string hmc_registr_util::type_nameA(DWORD theType)
{
	switch (theType)
	{
	case REG_NONE:
		return "REG_NONE";
	case REG_SZ:
		return "REG_SZ";
	case REG_EXPAND_SZ:
		return "REG_EXPAND_SZ";
	case REG_BINARY:
		return "REG_BINARY";
	case REG_DWORD:
		return "REG_DWORD";
	case REG_QWORD:
		return "REG_QWORD";
	case REG_LINK:
		return "REG_LINK";
	case REG_MULTI_SZ:
		return "REG_MULTI_SZ";
	case REG_RESOURCE_LIST:
		return "REG_RESOURCE_LIST";
	case REG_FULL_RESOURCE_DESCRIPTOR:
		return "REG_FULL_RESOURCE_DESCRIPTOR";
	case REG_RESOURCE_REQUIREMENTS_LIST:
		return "REG_RESOURCE_REQUIREMENTS_LIST";
	}

	return "";
}

std::wstring hmc_registr_util::type_nameW(DWORD theType)
{
	switch (theType)
	{
	case REG_NONE:
		return L"REG_NONE";
	case REG_SZ:
		return L"REG_SZ";
	case REG_EXPAND_SZ:
		return L"REG_EXPAND_SZ";
	case REG_BINARY:
		return L"REG_BINARY";
	case REG_DWORD:
		return L"REG_DWORD";
	case REG_QWORD:
		return L"REG_QWORD";
	case REG_LINK:
		return L"REG_LINK";
	case REG_MULTI_SZ:
		return L"REG_MULTI_SZ";
	case REG_RESOURCE_LIST:
		return L"REG_RESOURCE_LIST";
	case REG_FULL_RESOURCE_DESCRIPTOR:
		return L"REG_FULL_RESOURCE_DESCRIPTOR";
	case REG_RESOURCE_REQUIREMENTS_LIST:
		return L"REG_RESOURCE_REQUIREMENTS_LIST";
	}

	return L"";
}

bool hmc_registr_util::hasRegistrKey(HKEY hKey, std::string FolderPath, std::string key)
{
	return getValueStat(hKey, FolderPath, key).exists;
}

bool hmc_registr_util::hasRegistrKey(HKEY hKey, std::wstring FolderPath, std::wstring key)
{
	return getValueStat(hKey, FolderPath, key).exists;
}

bool hmc_registr_util::hasRegistrValue(HKEY hKey, std::string FolderPath, std::string key)
{
	auto valueStat = getValueStat(hKey, FolderPath, key);
	return valueStat.exists && valueStat.size > 0;
}

bool hmc_registr_util::hasRegistrValue(HKEY hKey, std::wstring FolderPath, std::wstring key)
{
	auto valueStat = getValueStat(hKey, FolderPath, key);
	return valueStat.exists && valueStat.size > 0;
}

bool hmc_registr_util::hasRegistrFolder(HKEY hKey, std::string FolderPath)
{
	bool result = false;
	HKEY open_hkey;
	DWORD openResult = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);
	HMC_FreeRegKeyAuto(open_hkey);

	switch (openResult)
	{
	case ERROR_SUCCESS:
		return true;
	case ERROR_FILE_NOT_FOUND:
		return false;
	}
	return result;
}

bool hmc_registr_util::hasRegistrFolder(HKEY hKey, std::wstring FolderPath)
{
	bool result = false;
	HKEY open_hkey;
	DWORD openResult = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);
	HMC_FreeRegKeyAuto(open_hkey);

	switch (openResult)
	{
	case ERROR_SUCCESS:
		return true;
	case ERROR_FILE_NOT_FOUND:
		return false;
	}
	return result;
}

bool hmc_registr_util::removeRegistrFolder(HKEY hKey, std::wstring FolderPath, std::wstring FolderName, bool tree)
{
	if (tree)
	{
		return removeRegistrTree(hKey, FolderPath, FolderName);
	}

	bool result = false;
	HKEY open_hkey;
	DWORD openResult = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);
	HMC_FreeRegKeyAuto(open_hkey);

	if (openResult == ERROR_SUCCESS)
	{
		openResult = ::RegDeleteKeyExW(open_hkey, FolderName.c_str(), KEY_WOW64_64KEY, 0);
		return (openResult == ERROR_SUCCESS) || (::RegDeleteKeyExW(open_hkey, FolderName.c_str(), KEY_WOW64_32KEY, 0) == ERROR_SUCCESS);
	}

	return false;
}

bool hmc_registr_util::removeRegistrFolder(HKEY hKey, std::string FolderPath, std::string FolderName, bool tree)
{
	if (tree)
	{
		return removeRegistrTree(hKey, FolderPath, FolderName);
	}

	bool result = false;
	HKEY open_hkey;
	DWORD openResult = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);
	HMC_FreeRegKeyAuto(open_hkey);

	if (openResult == ERROR_SUCCESS)
	{
		openResult = ::RegDeleteKeyExA(open_hkey, FolderName.c_str(), KEY_WOW64_64KEY, 0);
		return (openResult == ERROR_SUCCESS) || (::RegDeleteKeyExA(open_hkey, FolderName.c_str(), KEY_WOW64_32KEY, 0) == ERROR_SUCCESS);
	}

	return false;
}

bool hmc_registr_util::removeRegistrValue(HKEY hKey, std::string FolderPath, std::string key)
{

	// 默认值 应该直接清空
	if (key.empty())
	{
		return removeRegistrKey(hKey, FolderPath, key);
	}

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		auto open_value = getValueStat(open_hkey, key);
		return ::RegSetValueExA(open_hkey, key.c_str(), 0, open_value.type, nullptr, 0) == ERROR_SUCCESS;
	}

	return false;
}

bool hmc_registr_util::removeRegistrValue(HKEY hKey, std::wstring FolderPath, std::wstring key)
{
	// 默认值 应该直接清空
	if (key.empty())
	{
		return removeRegistrKey(hKey, FolderPath, key);
	}

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		auto open_value = getValueStat(open_hkey, key);
		return ::RegSetValueExW(open_hkey, key.c_str(), 0, open_value.type, nullptr, 0) == ERROR_SUCCESS;
	}

	return false;
}

bool hmc_registr_util::removeRegistrKey(HKEY hKey, std::string FolderPath, std::string key)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		return ::RegDeleteValueA(open_hkey, key.c_str()) == ERROR_SUCCESS;
	}

	return false;
}

bool hmc_registr_util::removeRegistrKey(HKEY hKey, std::wstring FolderPath, std::wstring key)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		return ::RegDeleteValueW(open_hkey, key.c_str()) == ERROR_SUCCESS;
	}

	return false;
}

bool hmc_registr_util::removeRegistrTree(HKEY hKey, std::string subKey, std::string DirName)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		RegDeleteTreeA(open_hkey, DirName.c_str());
	}
	return hasRegistrFolder(hKey, subKey + "\\" + DirName) == false;
}

bool hmc_registr_util::removeRegistrTree(HKEY hKey, std::wstring subKey, std::wstring DirName)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		RegDeleteTreeW(open_hkey, DirName.c_str());
	}
	return hasRegistrFolder(hKey, subKey + L"\\" + DirName) == false;
}

HKEY hmc_registr_util::getHive(std::wstring hkey)
{
	if (hkey.find(L"HKEY_CURRENT_USER") != std::wstring::npos)
	{
		return HKEY_CURRENT_USER;
	}

	if (hkey.find(L"HKEY_LOCAL_MACHINE") != std::wstring::npos)
	{
		return HKEY_LOCAL_MACHINE;
	}

	if (hkey.find(L"HKEY_CLASSES_ROOT") != std::wstring::npos)
	{
		return HKEY_CLASSES_ROOT;
	}

	if (hkey.find(L"HKEY_USERS") != std::wstring::npos)
	{
		return HKEY_USERS;
	}

	if (hkey.find(L"HKEY_CURRENT_CONFIG") != std::wstring::npos)
	{
		return HKEY_CURRENT_CONFIG;
	}

	return NULL;
}

HKEY hmc_registr_util::getHive(std::string hkey)
{
	if (hkey.find("HKEY_CURRENT_USER") != std::string::npos)
	{
		return HKEY_CURRENT_USER;
	}

	if (hkey.find("HKEY_LOCAL_MACHINE") != std::string::npos)
	{
		return HKEY_LOCAL_MACHINE;
	}

	if (hkey.find("HKEY_CLASSES_ROOT") != std::string::npos)
	{
		return HKEY_CLASSES_ROOT;
	}

	if (hkey.find("HKEY_USERS") != std::string::npos)
	{
		return HKEY_USERS;
	}

	if (hkey.find("HKEY_CURRENT_CONFIG") != std::string::npos)
	{
		return HKEY_CURRENT_CONFIG;
	}

	return NULL;
}

std::string hmc_registr_util::getHiveA(HKEY hkey)
{
	if (hkey == HKEY_CURRENT_USER)
	{
		return "HKEY_CURRENT_USER";
	}

	if (hkey == HKEY_LOCAL_MACHINE)
	{
		return "HKEY_LOCAL_MACHINE";
	}

	if (hkey == HKEY_CLASSES_ROOT)
	{
		return "HKEY_CLASSES_ROOT";
	}

	if (hkey == HKEY_USERS)
	{
		return "HKEY_USERS";
	}

	if (hkey == HKEY_CURRENT_CONFIG)
	{
		return "HKEY_CURRENT_CONFIG";
	}

	return "";
}

std::wstring hmc_registr_util::getHiveW(HKEY hkey)
{
	if (hkey == HKEY_CURRENT_USER)
	{
		return L"HKEY_CURRENT_USER";
	}

	if (hkey == HKEY_LOCAL_MACHINE)
	{
		return L"HKEY_LOCAL_MACHINE";
	}

	if (hkey == HKEY_CLASSES_ROOT)
	{
		return L"HKEY_CLASSES_ROOT";
	}

	if (hkey == HKEY_USERS)
	{
		return L"HKEY_USERS";
	}

	if (hkey == HKEY_CURRENT_CONFIG)
	{
		return L"HKEY_CURRENT_CONFIG";
	}

	return L"";
}

bool hmc_registr_util::getHive(HKEY hkey, std::string &ptr_hkey)
{
	ptr_hkey.clear();
	ptr_hkey.append(getHiveA(hkey));
	return ptr_hkey.empty();
}

bool hmc_registr_util::getHive(HKEY hkey, std::wstring &ptr_hkey)
{
	ptr_hkey.clear();
	ptr_hkey.append(getHiveW(hkey));
	return ptr_hkey.empty();
}

std::string hmc_registr_util::getHive(HKEY hkey)
{
	return getHiveA(hkey);
}

hmc_registr_util::chValueStat hmc_registr_util::getValueStat(HKEY hKey, std::string subKey, std::string key)
{
	hmc_registr_util::chValueStat result = {0, 0, false};

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (::RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS)
	{
		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (open_hkey != NULL && RegQueryValueExA(open_hkey, key.c_str(), nullptr, &result.type, nullptr, &result.size) == ERROR_SUCCESS)
		{
			result.exists = true;
		}
	}

	return result;
}

hmc_registr_util::chValueStat hmc_registr_util::getValueStat(HKEY hKey, std::wstring subKey, std::wstring key)
{
	hmc_registr_util::chValueStat result = {0, 0, false};

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	if (::RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS)
	{
		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (open_hkey != NULL && RegQueryValueExW(open_hkey, key.c_str(), nullptr, &result.type, nullptr, &result.size) == ERROR_SUCCESS)
		{
			result.exists = true;
		}
	}

	return result;
}

hmc_registr_util::chValueStat hmc_registr_util::getValueStat(HKEY hKey, std::wstring key)
{
	hmc_registr_util::chValueStat result = {0, 0, false};

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (hKey != NULL && RegQueryValueExW(hKey, key.c_str(), nullptr, &result.type, nullptr, &result.size) == ERROR_SUCCESS)
	{
		result.exists = true;
	}
	return result;
}

hmc_registr_util::chValueStat hmc_registr_util::getValueStat(HKEY hKey, std::string key)
{
	hmc_registr_util::chValueStat result = {0, 0, false};

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (hKey != NULL && RegQueryValueExA(hKey, key.c_str(), nullptr, &result.type, nullptr, &result.size) == ERROR_SUCCESS)
	{
		result.exists = true;
	}
	return result;
}

hmc_registr_util::chValue hmc_registr_util::chValueItme::getValueInfo()
{
	auto result = hmc_registr_util::chValue{
		0,	  // 值的大小
		0,	  // 类型
		{},	  // 数据
		false // 此键是否存在
	};

	if (!isFolder)
	{
		result.exists = true;
		result.size = size;
		result.value = value;
		result.type = type;
	}

	return result;
}

std::vector<hmc_registr_util::chValueItmeA> hmc_registr_util::getRegistrValueItmeList(HKEY hKey, std::string subKey)
{
	std::vector<hmc_registr_util::chValueItmeA> result = {};

	auto queryFolderInfo = queryRegistrFolderInfo(hKey, subKey, true, true);

	for (size_t i = 0; i < queryFolderInfo.folderKeyList.size(); i++)
	{
		auto key = queryFolderInfo.folderKeyList[i];
		hmc_registr_util::chValueItmeA data;
		data.FolderPath = subKey;
		data.vkey = key.name;
		data.root = hKey;
		data.time = 0;
		data.isFolder = false;

		// 取值
		auto value_data = GetRegistrSourceValue(hKey, subKey, key.name);
		data.value = value_data.value;
		data.type = value_data.type;
		data.is_value = value_data.size > 0;
		data.size = value_data.size;

		result.emplace_back(data);
	}

	for (size_t i = 0; i < queryFolderInfo.valueKeyList.size(); i++)
	{
		auto key = queryFolderInfo.valueKeyList[i];
		hmc_registr_util::chValueItmeA data;
		data.FolderPath = subKey;
		data.vkey = key.name;
		data.root = hKey;
		data.value = {};
		data.type = 0;
		data.is_value = 0;

		// 有用的值
		data.size = queryFolderInfo.size;
		data.time = queryFolderInfo.time;
		data.isFolder = true;

		// 类型 在文件夹中此值将被作为 目录键数量
		data.type = queryFolderInfo.folderSize;

		result.emplace_back(data);
	}
	return result;
}

std::vector<hmc_registr_util::chValueItmeW> hmc_registr_util::getRegistrValueItmeList(HKEY hKey, std::wstring subKey)
{
	std::vector<hmc_registr_util::chValueItmeW> result = {};
	auto queryFolderInfo = queryRegistrFolderInfo(hKey, subKey, true, true);

	for (size_t i = 0; i < queryFolderInfo.folderKeyList.size(); i++)
	{
		auto key = queryFolderInfo.folderKeyList[i];
		hmc_registr_util::chValueItmeW data;
		data.FolderPath = subKey;
		data.vkey = key.name;
		data.root = hKey;
		data.time = 0;
		data.isFolder = false;

		// 取值
		auto value_data = GetRegistrSourceValue(hKey, subKey, key.name);
		data.value = value_data.value;
		data.type = value_data.type;
		data.is_value = value_data.size > 0;
		data.size = value_data.size;

		result.emplace_back(data);
	}

	for (size_t i = 0; i < queryFolderInfo.valueKeyList.size(); i++)
	{
		auto key = queryFolderInfo.valueKeyList[i];
		hmc_registr_util::chValueItmeW data;
		data.FolderPath = subKey;
		data.vkey = key.name;
		data.root = hKey;
		data.value = {};
		data.type = 0;
		data.is_value = 0;

		// 有用的值
		data.size = queryFolderInfo.size;
		data.time = queryFolderInfo.time;
		data.isFolder = true;

		// 类型 在文件夹中此值将被作为 目录键数量
		data.type = queryFolderInfo.folderSize;

		result.emplace_back(data);
	}

	return result;
}

hmc_registr_util::chValue hmc_registr_util::GetRegistrSourceValue(HKEY hKey, std::wstring FolderPath, std::wstring KeyName)
{
	hmc_registr_util::chValue result = {0, 0, {}, false};
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS)
	{
		return GetRegistrSourceValue(open_hkey, KeyName);
	}

	return result;
}

hmc_registr_util::chValue hmc_registr_util::GetRegistrSourceValue(HKEY hKey, std::string FolderPath, std::string KeyName)
{
	hmc_registr_util::chValue result = {0, 0, {}, false};
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS)
	{
		return GetRegistrSourceValue(open_hkey, KeyName);
	}

	return result;
}

hmc_registr_util::chValue hmc_registr_util::GetRegistrSourceValue(HKEY open_hkey, std::string KeyName)
{

	DWORD valueType;
	DWORD pDataSize = 0;
	hmc_registr_util::chValue result = {0, 0, {}, false};

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &valueType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{
		result.size = pDataSize;
		result.type = valueType;
		result.exists = true;
	}

	// push data
	if (result.size)
	{
		std::vector<BYTE> value_data;
		value_data.resize(pDataSize);

		if (::RegQueryValueExA(open_hkey, KeyName.c_str(), 0, &valueType, reinterpret_cast<BYTE *>(value_data.data()), &pDataSize) == ERROR_SUCCESS)
		{
			result.value.resize(pDataSize);
			for (size_t i = 0; i < pDataSize; i++)
			{
				result.value[i] = value_data[i];
			}
		}
	}

	return result;
}

hmc_registr_util::chValue hmc_registr_util::GetRegistrSourceValue(HKEY open_hkey, std::wstring KeyName)
{
	DWORD valueType;
	DWORD pDataSize = 0;
	hmc_registr_util::chValue result = {0, 0, {}, false};

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &valueType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{
		result.size = pDataSize;
		result.type = valueType;
		result.exists = true;
	}

	// push data
	if (result.size)
	{
		std::vector<BYTE> value_data;
		value_data.resize(pDataSize);

		if (::RegQueryValueExW(open_hkey, KeyName.c_str(), 0, &valueType, reinterpret_cast<BYTE *>(value_data.data()), &pDataSize) == ERROR_SUCCESS)
		{
			result.value.resize(pDataSize);
			for (size_t i = 0; i < pDataSize; i++)
			{
				result.value[i] = value_data[i];
			}
		}
	}

	return result;
}

bool hmc_registr_util::createRegistrFolder(HKEY hKey, std::string keyPath)
{
	HKEY open_hkey = nullptr;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	HMC_FreeRegKeyAuto(open_hkey);
	long resulte = ::RegCreateKeyExA(hKey, keyPath.c_str(), 0, NULL,
									 dwOptions, KEY_WRITE, NULL, &open_hkey, &dwDisposition);
	if (resulte != ERROR_SUCCESS)
	{
		return false;
	}
	return (dwDisposition == REG_OPENED_EXISTING_KEY || dwDisposition == REG_CREATED_NEW_KEY);
}

bool hmc_registr_util::createRegistrFolder(HKEY hKey, std::wstring keyPath)
{
	HKEY open_hkey = nullptr;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	HMC_FreeRegKeyAuto(open_hkey);

	long resulte = ::RegCreateKeyExW(hKey, keyPath.c_str(), 0, NULL,
									 dwOptions, KEY_WRITE, NULL, &open_hkey, &dwDisposition);
	if (resulte != ERROR_SUCCESS)
	{
		return false;
	}
	return (dwDisposition == REG_OPENED_EXISTING_KEY || dwDisposition == REG_CREATED_NEW_KEY);
}

bool hmc_registr_util::copyRegistrFolder(HKEY hKey, std::string sourcePath, std::string toPath)
{
	HKEY sourceHKey = nullptr;
	HKEY toHKey = nullptr;
	DWORD dwDisposition;

	HMC_FreeRegKeyAuto(sourceHKey);
	HMC_FreeRegKeyAuto(toHKey);

	if (ERROR_SUCCESS != ::RegOpenKeyExA(hKey, sourcePath.c_str(), 0, KEY_ALL_ACCESS, &sourceHKey))
	{
		return false;
	}

	if (ERROR_SUCCESS != ::RegCreateKeyExA(hKey, toPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &toHKey, &dwDisposition))
	{
		return false;
	}

	return (ERROR_SUCCESS == ::RegCopyTreeA(sourceHKey, nullptr, toHKey));
}

bool hmc_registr_util::copyRegistrFolder(HKEY hKey, std::wstring sourcePath, std::wstring toPath)
{
	HKEY sourceHKey = nullptr;
	HKEY toHKey = nullptr;
	DWORD dwDisposition;

	HMC_FreeRegKeyAuto(sourceHKey);
	HMC_FreeRegKeyAuto(toHKey);

	if (ERROR_SUCCESS != ::RegOpenKeyExW(hKey, sourcePath.c_str(), 0, KEY_ALL_ACCESS, &sourceHKey))
	{
		return false;
	}

	if (ERROR_SUCCESS != ::RegCreateKeyExW(hKey, toPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &toHKey, &dwDisposition))
	{
		return false;
	}

	return (ERROR_SUCCESS == ::RegCopyTreeW(sourceHKey, nullptr, toHKey));
}

bool hmc_registr_util::copyRegistrValue(HKEY hKey, std::string sourcePath, std::string sourceKey, std::string toPath, std::string toKey)
{
	return SetRegistrSourceValue(hKey, toPath, toKey, GetRegistrSourceValue(hKey, sourcePath, sourceKey));
}

bool hmc_registr_util::copyRegistrValue(HKEY hKey, std::wstring sourcePath, std::wstring sourceKey, std::wstring toPath, std::wstring toKey)
{
	return SetRegistrSourceValue(hKey, toPath, toKey, GetRegistrSourceValue(hKey, sourcePath, sourceKey));
}

std::wstring hmc_registr_util::joinPath(std::wstring root, std::vector<std::wstring> NameList)
{
	std::wstring result = std::wstring(root);

	// 预开劈内存 单次开辟更节省性能
	auto max_length = root.size();
	for (size_t i = 0; i < NameList.size(); i++)
	{
		max_length = max_length + NameList[i].size() + sizeof(wchar_t);
	}
	result.reserve(max_length);

	for (size_t i = 0; i < NameList.size(); i++)
	{
		auto name = NameList[i];
		if (!name.empty())
		{
			result.push_back(L'\\');
			result.append(name);
		}
	}
	return result;
}

std::string hmc_registr_util::joinPath(std::string root, std::vector<std::string> NameList)
{
	std::string result = std::string(root);

	// 预开劈内存 单次开辟更节省性能
	auto max_length = root.size();
	for (size_t i = 0; i < NameList.size(); i++)
	{
		max_length = max_length + NameList[i].size() + sizeof(char);
	}
	result.reserve(max_length);

	for (size_t i = 0; i < NameList.size(); i++)
	{
		auto name = NameList[i];
		if (!name.empty())
		{
			result.push_back('\\');
			result.append(name);
		}
	}
	return result;
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, chValue Value)
{
	return SetRegistrSourceValue(hKey, FolderPath, KeyName, Value.type, Value.value);
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::string FolderPath, std::string KeyName, chValue Value)
{
	return SetRegistrSourceValue(hKey, FolderPath, KeyName, Value.type, Value.value);
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, DWORD type, const std::vector<BYTE> &value)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	LSTATUS status = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);

	if (status == ERROR_FILE_NOT_FOUND)
	{
		createRegistrFolder(hKey, FolderPath);
		status = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey);
	}

	if (status == ERROR_SUCCESS)
	{
		return SetRegistrSourceValue(open_hkey, KeyName, type, value);
	}

	return false;
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::string FolderPath, std::string KeyName, DWORD type, const std::vector<BYTE> &value)
{
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);
	bool is_open = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS;

	if (is_open)
	{
		return SetRegistrSourceValue(open_hkey, KeyName, type, value);
	}

	return false;
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::wstring KeyName, DWORD type, const std::vector<BYTE> &value)
{
	return ERROR_SUCCESS == ::RegSetValueExW(hKey, KeyName.c_str(), 0, type, reinterpret_cast<const BYTE *>(value.data()), static_cast<DWORD>(value.size() * sizeof(BYTE)));
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::string KeyName, DWORD type, const std::vector<BYTE> &value)
{
	return ERROR_SUCCESS == ::RegSetValueExA(hKey, KeyName.c_str(), 0, type, reinterpret_cast<const BYTE *>(value.data()), static_cast<DWORD>(value.size() * sizeof(BYTE)));
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::wstring KeyName, chValue Value)
{
	return SetRegistrSourceValue(hKey, KeyName, Value.type, Value.value);
}

bool hmc_registr_util::SetRegistrSourceValue(HKEY hKey, std::string KeyName, chValue Value)
{
	return SetRegistrSourceValue(hKey, KeyName, Value.type, Value.value);
}

hmc_registr_util::OpenKeyToken::operator HKEY() const
{
	return token;
}

bool hmc_registr_util::OpenKeyToken::operator!() const
{
	return (token == nullptr);
}

hmc_registr_util::OpenKeyToken::operator bool() const
{
	return (token != nullptr);
}

hmc_registr_util::OpenKeyToken::OpenKeyToken(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, bool CreateNotKey)
{
	if ((status = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &token)) != ERROR_SUCCESS)
	{
		if (CreateNotKey)
		{

			DWORD dwOptions = REG_OPTION_NON_VOLATILE;
			DWORD dwDisposition;

			status = ::RegCreateKeyExW(hKey, FolderPath.c_str(), 0, NULL, dwOptions, KEY_WRITE, NULL, &token, &dwDisposition);

			if (status != ERROR_SUCCESS)
			{
				return;
			}

			if ((status = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &token)) != ERROR_SUCCESS)
			{
				return;
			}
		}
	}
}

hmc_registr_util::OpenKeyToken::OpenKeyToken(HKEY hKey, std::string FolderPath, std::string KeyName, bool CreateNotKey)
{
	if ((status = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &token)) != ERROR_SUCCESS)
	{
		if (CreateNotKey)
		{

			DWORD dwOptions = REG_OPTION_NON_VOLATILE;
			DWORD dwDisposition;

			status = ::RegCreateKeyExA(hKey, FolderPath.c_str(), 0, NULL, dwOptions, KEY_WRITE, NULL, &token, &dwDisposition);

			if (status != ERROR_SUCCESS)
			{
				return;
			}

			if ((status = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &token)) != ERROR_SUCCESS)
			{
				return;
			}
		}
	}
}

hmc_registr_util::OpenKeyToken::~OpenKeyToken()
{
	if (token != nullptr)
	{
		::RegCloseKey(token);
		token = nullptr;
	}
}

bool hmc_registr_util::SetRegistrString(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, const std::wstring &Input, bool expand)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = expand ? REG_EXPAND_SZ : REG_SZ;
	DWORD buff_size = static_cast<DWORD>(Input.size() * sizeof(wchar_t));

	if (::RegSetValueExW(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(Input.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExW(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrString(HKEY hKey, std::string FolderPath, std::string KeyName, const std::string &Input, bool expand)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = expand ? REG_EXPAND_SZ : REG_SZ;
	DWORD buff_size = static_cast<DWORD>(Input.size() * sizeof(char));

	if (::RegSetValueExA(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(Input.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExA(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrString(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, const std::wstring &Input, const DWORD retype)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD buff_size = static_cast<DWORD>(Input.size() * sizeof(wchar_t));

	if (::RegSetValueExW(token, KeyName.c_str(), NULL, retype, reinterpret_cast<const BYTE *>(Input.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExW(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == retype && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrString(HKEY hKey, std::string FolderPath, std::string KeyName, const std::string &Input, const DWORD retype)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD buff_size = static_cast<DWORD>(Input.size() * sizeof(char));

	if (::RegSetValueExA(token, KeyName.c_str(), NULL, retype, reinterpret_cast<const BYTE *>(Input.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExA(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == retype && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrMulti(HKEY hKey, std::string FolderPath, std::string KeyName, const std::vector<std::string> &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_MULTI_SZ;
	DWORD buff_size = 0; // static_cast<DWORD>(Input.size() * sizeof(char));

	// 计算总大小
	size_t length = Input.size();

	for (size_t i = 0; i < length; i++)
	{
		auto it = &Input.at(i);
		buff_size += static_cast<DWORD>(it->size() * sizeof(char));
	}

	std::string input_multi = "";
	input_multi.reserve(buff_size / sizeof(char));

	length = Input.size();

	// 制作数组
	for (size_t i = 0; i < length; i++)
	{
		auto it = &Input.at(i);
		size_t it_size = it->size();

		for (size_t si = 0; si < it_size; si++)
		{
			auto it2 = it->at(si);

			if (si + 1 < it_size && it2 == '\0')
			{
				input_multi.append("\\0");
			}
			else
			{
				input_multi.push_back(it2);
			}
		}

		auto end_pos = input_multi.find_last_not_of('\0');
		if (end_pos == std::string::npos)
		{
			input_multi.push_back('\0');
		}
	}

	// 以\0\0 结尾
	input_multi.push_back('\0');

	buff_size = static_cast<DWORD>(input_multi.size() * sizeof(char));

	if (::RegSetValueExA(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(input_multi.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExA(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrMulti(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, const std::vector<std::wstring> &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_MULTI_SZ;
	DWORD buff_size = 0; // static_cast<DWORD>(Input.size() * sizeof(char));

	// 计算总大小
	size_t length = Input.size();

	for (size_t i = 0; i < length; i++)
	{
		auto it = &Input.at(i);
		buff_size += static_cast<DWORD>(it->size() * sizeof(wchar_t));
	}

	std::wstring input_multi = L"";
	input_multi.reserve(buff_size / sizeof(wchar_t));

	length = Input.size();

	// 制作数组
	for (size_t i = 0; i < length; i++)
	{
		auto it = &Input.at(i);
		size_t it_size = it->size();

		for (size_t si = 0; si < it_size; si++)
		{

			auto it2 = it->at(si);

			if (si + 1 < it_size && it2 == L'\0')
			{
				input_multi.append(L"\\0");
			}
			else
			{
				input_multi.push_back(it2);
			}
		}

		auto end_pos = input_multi.find_last_not_of(L'\0');
		if (end_pos == std::string::npos)
		{
			input_multi.push_back(L'\0');
		}
	}

	// 以\0\0 结尾
	input_multi.push_back(L'\0');

	buff_size = static_cast<DWORD>(input_multi.size() * sizeof(wchar_t));

	if (::RegSetValueExW(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(input_multi.c_str()), buff_size) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExW(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrInt32(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, const std::uint32_t &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_DWORD;
	DWORD size_ptr = sizeof(std::uint32_t);
	std::uint32_t input = Input;

	if (::RegSetValueExW(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(&input), size_ptr) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExW(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrInt32(HKEY hKey, std::string FolderPath, std::string KeyName, const std::uint32_t &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_DWORD;
	DWORD size_ptr = sizeof(std::uint32_t);
	std::uint32_t input = Input;

	if (::RegSetValueExA(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(&input), size_ptr) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExA(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrInt64(HKEY hKey, std::string FolderPath, std::string KeyName, const std::uint64_t &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_QWORD;
	DWORD size_ptr = sizeof(std::uint64_t);
	std::uint64_t input = Input;

	if (::RegSetValueExA(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(&input), size_ptr) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExA(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

bool hmc_registr_util::SetRegistrInt64(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, const std::uint64_t &Input)
{
	bool result = false;
	OpenKeyToken token = OpenKeyToken(hKey, FolderPath, KeyName, true);

	if (!token)
	{
		return result;
	}

	DWORD to_type = REG_QWORD;
	DWORD size_ptr = sizeof(std::uint64_t);
	std::uint64_t input = Input;

	if (::RegSetValueExW(token, KeyName.c_str(), NULL, to_type, reinterpret_cast<const BYTE *>(&input), size_ptr) == ERROR_SUCCESS)
	{

		DWORD lpType = REG_NONE;
		DWORD pDataSize = 0;

		if (::RegQueryValueExW(token, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			result = lpType == to_type && pDataSize != 0;
			return result;
		}
	}

	return result;
}

std::vector<std::wstring> hmc_registr_util::GetRegistrMulti(HKEY hKey, std::wstring FolderPath, std::wstring KeyName)
{

	std::vector<std::wstring> list = {};

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return list;
	}

	DWORD lpType = REG_MULTI_SZ;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{
		std::wstring lpWstrList;
		lpWstrList.resize(pDataSize / sizeof(wchar_t));

		if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpWstrList[0]), &pDataSize) == ERROR_SUCCESS)
		{
			std::wstring temp = L"";

			auto end_pos = lpWstrList.size();

			// 移除所有尾部 \0
			while ((end_pos = lpWstrList.find_last_not_of(L'\0')) != lpWstrList.size() - 1)
			{
				lpWstrList.erase(end_pos + 1);
			}

			size_t len = lpWstrList.size();

			for (size_t i = 0; i < len; i++)
			{
				wchar_t it = lpWstrList[i];
				if (it == L'\0')
				{
					list.push_back(std::wstring(temp + L""));
					temp.clear();
					continue;
				}

				temp.push_back(it);
			}

			if (!temp.empty())
			{
				list.push_back(std::wstring(temp + L""));
				temp.clear();
			}
		}
	}
	return list;
}

std::vector<std::string> hmc_registr_util::GetRegistrMulti(HKEY hKey, std::string FolderPath, std::string KeyName)
{

	std::vector<std::string> list = {};

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return list;
	}

	DWORD lpType = REG_MULTI_SZ;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{
		std::string lpWstrList;
		lpWstrList.resize(pDataSize / sizeof(char));

		if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpWstrList[0]), &pDataSize) == ERROR_SUCCESS)
		{
			std::string temp = "";

			auto end_pos = lpWstrList.size();

			// 移除所有尾部 \0
			while ((end_pos = lpWstrList.find_last_not_of('\0')) != lpWstrList.size() - 1)
			{
				lpWstrList.erase(end_pos + 1);
			}

			size_t len = lpWstrList.size();

			for (size_t i = 0; i < len; i++)
			{
				char it = lpWstrList[i];
				if (it == '\0')
				{
					list.push_back(std::string(temp + ""));
					temp.clear();
					continue;
				}

				temp.push_back(it);
			}

			if (!temp.empty())
			{
				list.push_back(std::string(temp + ""));
				temp.clear();
			}
		}
	}
	return list;
}

std::wstring hmc_registr_util::GetRegistrString(HKEY hKey, std::wstring FolderPath, std::wstring KeyName, bool expand)
{

	std::wstring result = L"";

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = expand ? RRF_RT_REG_EXPAND_SZ : REG_SZ;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{

			if (pDataSize < 1)
			{
				return result;
			}

			size_t char_len = pDataSize / sizeof(wchar_t);

			std::unique_ptr<wchar_t[]> lpValue (new (std::nothrow) wchar_t[char_len]);

			if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(lpValue.get()), &pDataSize) == ERROR_SUCCESS)
			{
				char_len = pDataSize / sizeof(wchar_t);
				result.reserve(char_len);

				for (size_t i = 0; i < char_len; i++)
				{
					auto it = lpValue[i];
					if (it == L'\0')
					{
						break;
					}
					result.push_back(it);
				}
			}
		}
	}

	// 内存预开劈过了 克隆新的对象
	if (result.capacity() > 0 && result.capacity() != result.size())
	{
		return std::wstring(result.c_str());
	}

	return result;
}

std::string hmc_registr_util::GetRegistrString(HKEY hKey, std::string FolderPath, std::string KeyName, bool expand)
{

	std::string result = {};

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = expand ? RRF_RT_REG_EXPAND_SZ : REG_SZ;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		if (pDataSize < 1)
		{
			return result;
		}

		size_t char_len = pDataSize / sizeof(char);

		std::unique_ptr<char[]> lpValue (new (std::nothrow) char[char_len]);

		if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(lpValue.get()), &pDataSize) == ERROR_SUCCESS)
		{

			char_len = pDataSize / sizeof(char);
			result.reserve(char_len);

			for (size_t i = 0; i < char_len; i++)
			{
				auto it = lpValue[i];
				if (it == '\0')
				{
					break;
				}
				result.push_back(it);
			}
		}
	}

	// 内存预开劈过了 克隆新的对象
	if (result.capacity() > 0 && result.capacity() != result.size())
	{
		return std::string(result.c_str());
	}

	return result;
}

std::uint32_t hmc_registr_util::GetRegistrInt32(HKEY hKey, std::wstring FolderPath, std::wstring KeyName)
{
	std::uint32_t result = 0;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = REG_DWORD;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{
			if (lpType == REG_QWORD)
			{
				std::uint64_t lpValue = 0;

				if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpValue), &pDataSize) == ERROR_SUCCESS)
				{
					result = static_cast<std::uint32_t>(lpValue);
					return result;
				}
			}

			if (pDataSize < 1 || pDataSize > sizeof(std::uint32_t))
			{
				return result;
			}

			std::uint32_t lpValue = 0;

			if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpValue), &pDataSize) == ERROR_SUCCESS)
			{
				result = lpValue;
				return result;
			}
		}
	}

	return result;
}

std::uint32_t hmc_registr_util::GetRegistrInt32(HKEY hKey, std::string FolderPath, std::string KeyName)
{
	std::uint32_t result = 0;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = REG_DWORD;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{

			if (pDataSize < 1 || pDataSize > sizeof(std::uint32_t))
			{
				return result;
			}

			std::uint32_t lpValue = 0;

			if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpValue), &pDataSize) == ERROR_SUCCESS)
			{
				result = lpValue;
				return result;
			}
		}
	}

	return result;
}

std::uint64_t hmc_registr_util::GetRegistrInt64(HKEY hKey, std::wstring FolderPath, std::wstring KeyName)
{
	std::uint64_t result = 0;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = REG_QWORD;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{

			if (pDataSize < 1 || pDataSize > sizeof(std::uint64_t))
			{
				return result;
			}

			std::uint64_t lpValue = 0;

			if (::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpValue), &pDataSize) == ERROR_SUCCESS)
			{
				result = lpValue;
				return result;
			}
		}
	}

	return result;
}

std::uint64_t hmc_registr_util::GetRegistrInt64(HKEY hKey, std::string FolderPath, std::string KeyName)
{
	std::uint64_t result = 0;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD lpType = REG_QWORD;
	DWORD pDataSize = 0;

	// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
	if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
	{

		// 第一次调用 RegQueryValueEx 获取值的大小，放入 dataSize 变量中
		if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, nullptr, &pDataSize) == ERROR_SUCCESS)
		{

			if (pDataSize < 1 || pDataSize > sizeof(std::uint64_t))
			{
				return result;
			}

			std::uint64_t lpValue = 0;

			if (::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &lpType, reinterpret_cast<BYTE *>(&lpValue), &pDataSize) == ERROR_SUCCESS)
			{
				result = lpValue;
				return result;
			}
		}
	}

	return result;
}

bool hmc_registr_util::removeRegistrTree(HKEY hKey, std::string subKey)
{
	bool result = false;
	auto splitSubKey = hmc_string_util::split(subKey, '\\');

	if (splitSubKey.size() <= 1)
	{
		return result;
	}

	auto Key = splitSubKey.back();
	splitSubKey.pop_back();

	auto cloneSubKey = hmc_string_util::join(splitSubKey, '\\');
	result = removeRegistrTree(hKey, cloneSubKey, Key);
	return result;
}

bool hmc_registr_util::removeRegistrTree(HKEY hKey, std::wstring subKey)
{
	bool result = false;
	auto splitSubKey = hmc_string_util::split(subKey, '\\');

	if (splitSubKey.size() <= 1)
	{
		return result;
	}

	auto Key = splitSubKey.back();
	splitSubKey.pop_back();

	auto cloneSubKey = hmc_string_util::join(splitSubKey, '\\');

	return removeRegistrTree(hKey, cloneSubKey, Key);
}

bool hmc_registr_util::removeRegistrFolder(HKEY hKey, std::wstring FolderPath, bool tree)
{
	bool result = false;
	auto splitSubKey = hmc_string_util::split(FolderPath, '\\');

	if (splitSubKey.size() <= 1)
	{
		return result;
	}

	auto Key = splitSubKey.back();
	splitSubKey.pop_back();

	auto cloneSubKey = hmc_string_util::join(splitSubKey, '\\');

	return removeRegistrFolder(hKey, cloneSubKey, Key, tree);
}

bool hmc_registr_util::removeRegistrFolder(HKEY hKey, std::string FolderPath, bool tree)
{
	bool result = false;
	auto splitSubKey = hmc_string_util::split(FolderPath, '\\');

	if (splitSubKey.size() <= 1)
	{
		return result;
	}

	auto Key = splitSubKey.back();
	splitSubKey.pop_back();

	auto cloneSubKey = hmc_string_util::join(splitSubKey, '\\');

	return removeRegistrFolder(hKey, cloneSubKey, Key, tree);
}

bool hmc_registr_util::removeRegistrValue(HKEY hKey, const std::wstring FolderPath, const std::vector<std::wstring> keyList)
{
	bool result = false;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : keyList)
		{
			auto open_value = getValueStat(open_hkey, key);
			if (open_value.exists)
			{
				bool temp = ::RegSetValueExW(open_hkey, key.c_str(), 0, open_value.type, nullptr, 0) == ERROR_SUCCESS;
				result = result ? result : temp;
			}
		}
	}

	return result;
}

bool hmc_registr_util::removeRegistrValue(HKEY hKey, const std::string FolderPath, const std::vector<std::string> keyList)
{
	bool result = false;

	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : keyList)
		{
			auto open_value = getValueStat(open_hkey, key);
			if (open_value.exists)
			{
				bool temp = ::RegSetValueExA(open_hkey, key.c_str(), 0, open_value.type, nullptr, 0) == ERROR_SUCCESS;
				result = result ? result : temp;
			}
		}
	}

	return result;
}

bool hmc_registr_util::removeRegistrKey(HKEY hKey, const std::wstring FolderPath, const std::vector<std::wstring> keyList)
{
	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : keyList)
		{
			bool temp = ::RegDeleteValueW(open_hkey, key.c_str()) == ERROR_SUCCESS;
			result = result ? result : temp;
		}
	}

	return result;
}

bool hmc_registr_util::removeRegistrKey(HKEY hKey, const std::string FolderPath, const std::vector<std::string> keyList)
{
	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : keyList)
		{
			bool temp = ::RegDeleteValueA(open_hkey, key.c_str()) == ERROR_SUCCESS;
			result = result ? result : temp;
		}
	}

	return result;
}

bool hmc_registr_util::createRegistrValueKey(HKEY hKey, const std::string FolderPath, const std::map<std::string, DWORD> ValueKeyList)
{

	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : ValueKeyList)
		{
			const std::string first = key.first;
			const DWORD second = key.second;

			bool temp = ::RegSetValueExA(open_hkey, first.c_str(), 0, second, nullptr, 0) == ERROR_SUCCESS;
			result = result ? result : temp;
		}
	}

	return result;
}

bool hmc_registr_util::createRegistrValueKey(HKEY hKey, const std::wstring FolderPath, const std::map<std::wstring, DWORD> ValueKeyList)
{
	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		for (auto &&key : ValueKeyList)
		{
			const std::wstring first = key.first;
			const DWORD second = key.second;

			bool temp = ::RegSetValueExW(open_hkey, first.c_str(), 0, second, nullptr, 0) == ERROR_SUCCESS;
			result = result ? result : temp;
		}
	}

	return result;
}

hmc_registr_util::chQueryFolderInfoA::chQueryFolderInfoA()
{
	valueKeyList = {};
	folderKeyList = {};
	size = 0i32;			 // 此所有键总数量
	exists = false;			 // 此键是否存在
	folderSize = 0i32;		 // 此目录键总数量
	keySize = 0i32;			 // 此目录键总数量
	time = 0ll;				 // 时间戳
	maxValueDataSize = 0i32; // 最长值数据的大小
}

hmc_registr_util::chQueryFolderInfoW::chQueryFolderInfoW()
{
	valueKeyList = {};
	folderKeyList = {};
	size = 0i32;			 // 此所有键总数量
	exists = false;			 // 此键是否存在
	folderSize = 0i32;		 // 此目录键总数量
	keySize = 0i32;			 // 此目录键总数量
	time = 0ll;				 // 时间戳
	maxValueDataSize = 0i32; // 最长值数据的大小
}

std::wstring hmc_registr_util::chQueryFolderInfoW::to_json()
{
	return to_json(*this);
}

std::wstring hmc_registr_util::chQueryFolderInfoW::to_json(hmc_registr_util::chQueryFolderInfoW QueryFolderInfo)
{
	const wchar_t template_data_ValueKey[46] = L"{"
											   L"\"name\":\"{name}\","
											   L"\"type\":{type},"
											   L"\"size\":{size}"
											   L"}";

	const wchar_t template_data_FolderKey[32] = L"{"
												L"\"name\":\"{name}\","
												L"\"time\":{time}"
												L"}";

	const wchar_t template_data[148] =
		L"{"
		L"\"size\":{size},"
		L"\"exists\":{exists},"
		L"\"folderSize\":{folderSize},"
		L"\"keySize\":{keySize},"
		L"\"time\":{time},"
		L"\"maxValueDataSize\":{maxValueDataSize},";
	// L"\"folderKeyList\":{folderKeyList}"
	// L"}";

	std::wstring result = std::wstring();

	size_t size = sizeof(template_data);
	size_t folderKeyList_size = 0;
	size_t valueKeyList_size = 0;

	for (size_t i = 0; i < QueryFolderInfo.folderKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.folderKeyList.at(i);
		folderKeyList_size = at.name.size() + MAX_PATH;
	}

	for (size_t i = 0; i < QueryFolderInfo.valueKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.valueKeyList.at(i);
		valueKeyList_size = at.name.size() + MAX_PATH;
	}

	size = size + folderKeyList_size + valueKeyList_size;
	result.reserve(size);
	result.append(template_data);

	hmc_string_util::replace(result, L"{size}", std::to_wstring(QueryFolderInfo.size));
	hmc_string_util::replace(result, L"{exists}", std::to_wstring(QueryFolderInfo.exists));
	hmc_string_util::replace(result, L"{folderSize}", std::to_wstring(QueryFolderInfo.folderSize));
	hmc_string_util::replace(result, L"{keySize}", std::to_wstring(QueryFolderInfo.keySize));
	hmc_string_util::replace(result, L"{time}", std::to_wstring(QueryFolderInfo.time));
	hmc_string_util::replace(result, L"{maxValueDataSize}", std::to_wstring(QueryFolderInfo.maxValueDataSize));

	result.append(L"\"valueKeyList\":[");
	for (size_t i = 0; i < QueryFolderInfo.valueKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.valueKeyList.at(i);

		result.append(L"{\"name\":");
		result.append(hmc_json_util::escapeJsonString(at.name, true));
		result.push_back(',');
		result.append(L"\"type\":");
		result.append(std::to_wstring(at.type));
		result.append(L",\"type_name\":\"");
		result.append(hmc_registr_util::type_nameW(at.type));
		result.append(L"\",\"size\":");
		result.append(std::to_wstring(at.size));
		result.push_back('}');

		if (i + 1 < QueryFolderInfo.valueKeyList.size())
		{
			result.push_back(',');
		}
	}
	result.push_back(']');
	result.push_back(',');

	result.append(L"\"folderKeyList\":[");
	for (size_t i = 0; i < QueryFolderInfo.folderKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.folderKeyList.at(i);

		result.append(L"{\"name\":");
		result.append(hmc_json_util::escapeJsonString(at.name, true));
		result.push_back(',');
		result.append(L"\"time\":");
		result.append(std::to_wstring(at.time));
		result.push_back('}');

		if (i + 1 < QueryFolderInfo.folderKeyList.size())
		{
			result.push_back(',');
		}
	}

	result.push_back(']');

	result.push_back('}');

	hmc_string_util::removeAllCharPtr(result, '\0');
	return result;
}

std::string hmc_registr_util::chQueryFolderInfoA::to_json()
{
	return to_json(*this);
}

std::string hmc_registr_util::chQueryFolderInfoA::to_json(hmc_registr_util::chQueryFolderInfoA QueryFolderInfo)
{
	const char template_data_ValueKey[46] = "{"
											"\"name\":\"{name}\","
											"\"type\":{type},"
											"\"size\":{size}"
											"}";

	const char template_data_FolderKey[32] = "{"
											 "\"name\":\"{name}\","
											 "\"time\":{time}"
											 "}";

	const char template_data[148] =
		"{"
		"\"size\":{size},"
		"\"exists\":{exists},"
		"\"folderSize\":{folderSize},"
		"\"keySize\":{keySize},"
		"\"time\":{time},"
		"\"maxValueDataSize\":{maxValueDataSize},";
	// "\"folderKeyList\":{folderKeyList}"
	// "}";

	std::string result = std::string();

	size_t size = sizeof(template_data);
	size_t folderKeyList_size = 0;
	size_t valueKeyList_size = 0;

	for (size_t i = 0; i < QueryFolderInfo.folderKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.folderKeyList.at(i);
		folderKeyList_size = at.name.size() + MAX_PATH;
	}

	for (size_t i = 0; i < QueryFolderInfo.valueKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.valueKeyList.at(i);
		valueKeyList_size = at.name.size() + MAX_PATH;
	}

	size = size + folderKeyList_size + valueKeyList_size;
	result.reserve(size);
	result.append(template_data);

	hmc_string_util::replace(result, "{size}", std::to_string(QueryFolderInfo.size));
	hmc_string_util::replace(result, "{exists}", std::to_string(QueryFolderInfo.exists));
	hmc_string_util::replace(result, "{folderSize}", std::to_string(QueryFolderInfo.folderSize));
	hmc_string_util::replace(result, "{keySize}", std::to_string(QueryFolderInfo.keySize));
	hmc_string_util::replace(result, "{time}", std::to_string(QueryFolderInfo.time));
	hmc_string_util::replace(result, "{maxValueDataSize}", std::to_string(QueryFolderInfo.maxValueDataSize));

	result.append("\"valueKeyList\":[");
	for (size_t i = 0; i < QueryFolderInfo.valueKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.valueKeyList.at(i);

		result.append("{\"name\":");
		result.append(hmc_json_util::escapeJsonString(at.name, true));
		result.push_back(',');
		result.append("\"type\":");
		result.append(std::to_string(at.type));
		result.append(",\"type_name\":\"");
		result.append(hmc_registr_util::type_nameA(at.type));
		result.append("\",\"size\":");
		result.append(std::to_string(at.size));
		result.push_back('}');

		if (i + 1 < QueryFolderInfo.valueKeyList.size())
		{
			result.push_back(',');
		}
	}
	result.push_back(']');
	result.push_back(',');

	result.append("\"folderKeyList\":[");
	for (size_t i = 0; i < QueryFolderInfo.folderKeyList.size(); i++)
	{
		auto &at = QueryFolderInfo.folderKeyList.at(i);

		result.append("{\"name\":");
		result.append(hmc_json_util::escapeJsonString(at.name, true));
		result.push_back(',');
		result.append("\"time\":");
		result.append(std::to_string(at.time));
		result.push_back('}');

		if (i + 1 < QueryFolderInfo.folderKeyList.size())
		{
			result.push_back(',');
		}
	}

	result.push_back(']');

	result.push_back('}');

	hmc_string_util::removeAllCharPtr(result, '\0');

	return result;
}

hmc_registr_util::chQueryFolderInfoA hmc_registr_util::queryRegistrFolderInfo(HKEY hKey, std::string FolderPath, bool isQueryFolderKey, bool isQueryValueKey)
{

	chQueryFolderInfoA result = chQueryFolderInfoA();

	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	DWORD cchClassName = MAX_PATH;	// 类字符串的大小
	DWORD cSubKeys = 0;				// 子键数
	DWORD cbMaxSubKey = 0;			// 最长子键大小
	DWORD cchMaxClass = 0;			// 最长类字符串
	DWORD cValues = 0;				// 键值的个数
	DWORD cchMaxValue = 0;			// 最长值名
	DWORD cbMaxValueData = 0;		// 最长值数据
	DWORD cbSecurityDescriptor = 0; // 安全描述符的大小
	FILETIME ftLastWriteTime{};		// 最后写入时间

	// wchar_t achValue[MAX_VALUE_NAME]; // key存储

	DWORD cchValueLeng = MAX_VALUE_NAME; // 数据序号
	std::string achValue;

	DWORD retCode = ERROR_SUCCESS;

	// 获取key数量 以及预分配内存大小
	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{

		retCode = ::RegQueryInfoKeyA(
			open_hkey,			   // key句柄
			nullptr,			   // 类名缓冲区
			nullptr,			   // 类字符串的大小
			nullptr,			   // 无
			&cSubKeys,			   // 子键数
			&cbMaxSubKey,		   // 最长子键大小
			&cchMaxClass,		   // 最长类字符串
			&cValues,			   // 键值的个数
			&cchMaxValue,		   // 最长值名
			&cbMaxValueData,	   // 最长值数据
			&cbSecurityDescriptor, // 安全描述符的大小
			&ftLastWriteTime);	   // 最后写入时间
	}

	if (ERROR_SUCCESS == retCode)
	{
		result.size = cSubKeys + cValues;
		result.exists = true;
		result.folderSize = cSubKeys;
		result.keySize = cValues;
		result.time = fileTimeToTimeStamp(ftLastWriteTime);
		result.maxValueDataSize = cbMaxValueData;

		if (isQueryFolderKey || isQueryValueKey)
		{
			achValue.reserve(cchValueLeng);
			achValue.resize(cchValueLeng);
		}

		// 枚举子键
		if (cSubKeys && isQueryFolderKey)
		{

			FILETIME ftLastWriteTime_temp{}; // 最后写入时间

			for (DWORD index = 0; index < cSubKeys; index++)
			{

				retCode = ::RegEnumKeyExA(open_hkey, index,
										  achValue.data(),
										  &cchValueLeng,
										  NULL,
										  NULL,
										  NULL,
										  &ftLastWriteTime_temp);

				if (retCode == ERROR_SUCCESS && cchValueLeng != 0)
				{
					achValue.resize(cchValueLeng);

					chQueryFolderKeyItemA data = {std::string(), fileTimeToTimeStamp(ftLastWriteTime_temp)};
					data.name.append(achValue.begin(), achValue.end());

					result.folderKeyList.push_back(std::move(data));
				}

				retCode = ERROR_SUCCESS;
				cchValueLeng = MAX_KEY_LENGTH;
				achValue.clear();
				achValue.resize(MAX_KEY_LENGTH);
			}
		}

		// 枚举键值。
		if (cValues && isQueryValueKey)
		{

			for (DWORD index = 0; index < cValues; index++)
			{

				DWORD lpType = REG_NONE;
				DWORD lpcbDataSize = 0;

				retCode = ::RegEnumValueA(open_hkey, index, achValue.data(), &cchValueLeng, NULL, &lpType, NULL, &lpcbDataSize);

				if (retCode == ERROR_SUCCESS)
				{
					achValue.resize(cchValueLeng);

					chQueryValueKeyItemA data = {std::string(), lpType, lpcbDataSize};
					data.name.append(achValue.begin(), achValue.end());

					result.valueKeyList.push_back(std::move(data));
				}

				retCode = ERROR_SUCCESS;
				cchValueLeng = MAX_KEY_LENGTH;
				achValue.clear();
				achValue.resize(MAX_KEY_LENGTH);
			}
		}
	}

	return result;
}

hmc_registr_util::chQueryFolderInfoW hmc_registr_util::queryRegistrFolderInfo(HKEY hKey, std::wstring FolderPath, bool isQueryFolderKey, bool isQueryValueKey)
{

	chQueryFolderInfoW result = chQueryFolderInfoW();

	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	DWORD cchClassName = MAX_PATH;	// 类字符串的大小
	DWORD cSubKeys = 0;				// 子键数
	DWORD cbMaxSubKey = 0;			// 最长子键大小
	DWORD cchMaxClass = 0;			// 最长类字符串
	DWORD cValues = 0;				// 键值的个数
	DWORD cchMaxValue = 0;			// 最长值名
	DWORD cbMaxValueData = 0;		// 最长值数据
	DWORD cbSecurityDescriptor = 0; // 安全描述符的大小
	FILETIME ftLastWriteTime{};		// 最后写入时间

	// wchar_t achValue[MAX_VALUE_NAME]; // key存储

	DWORD cchValueLeng = MAX_VALUE_NAME; // 数据序号
	std::wstring achValue;

	DWORD retCode = ERROR_SUCCESS;

	// 获取key数量 以及预分配内存大小
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{

		retCode = ::RegQueryInfoKeyW(
			open_hkey,			   // key句柄
			nullptr,			   // 类名缓冲区
			nullptr,			   // 类字符串的大小
			nullptr,			   // 无
			&cSubKeys,			   // 子键数
			&cbMaxSubKey,		   // 最长子键大小
			&cchMaxClass,		   // 最长类字符串
			&cValues,			   // 键值的个数
			&cchMaxValue,		   // 最长值名
			&cbMaxValueData,	   // 最长值数据
			&cbSecurityDescriptor, // 安全描述符的大小
			&ftLastWriteTime);	   // 最后写入时间
	}

	if (ERROR_SUCCESS == retCode)
	{
		result.size = cSubKeys + cValues;
		result.exists = true;
		result.folderSize = cSubKeys;
		result.keySize = cValues;
		result.time = fileTimeToTimeStamp(ftLastWriteTime);
		result.maxValueDataSize = cbMaxValueData;

		if (isQueryFolderKey || isQueryValueKey)
		{
			achValue.reserve(cchValueLeng);
			achValue.resize(cchValueLeng);
		}

		// 枚举子键
		if (cSubKeys && isQueryFolderKey)
		{

			FILETIME ftLastWriteTime_temp{}; // 最后写入时间

			for (DWORD index = 0; index < cSubKeys; index++)
			{

				retCode = ::RegEnumKeyExW(open_hkey, index,
										  achValue.data(),
										  &cchValueLeng,
										  NULL,
										  NULL,
										  NULL,
										  &ftLastWriteTime_temp);

				if (retCode == ERROR_SUCCESS && cchValueLeng != 0)
				{
					achValue.resize(cchValueLeng);

					chQueryFolderKeyItemW data = {std::wstring(), fileTimeToTimeStamp(ftLastWriteTime_temp)};
					data.name.append(achValue.begin(), achValue.end());

					result.folderKeyList.push_back(std::move(data));
				}

				retCode = ERROR_SUCCESS;
				cchValueLeng = MAX_KEY_LENGTH;
				achValue.clear();
				achValue.resize(MAX_KEY_LENGTH);
			}
		}

		// 枚举键值。
		if (cValues && isQueryValueKey)
		{

			for (DWORD index = 0; index < cValues; index++)
			{

				DWORD lpType = REG_NONE;
				DWORD lpcbDataSize = 0;

				retCode = ::RegEnumValueW(open_hkey, index, achValue.data(), &cchValueLeng, NULL, &lpType, NULL, &lpcbDataSize);

				if (retCode == ERROR_SUCCESS)
				{
					achValue.resize(cchValueLeng);

					chQueryValueKeyItemW data = {std::wstring(), lpType, lpcbDataSize};
					data.name.append(achValue.begin(), achValue.end());

					result.valueKeyList.push_back(std::move(data));
				}

				retCode = ERROR_SUCCESS;
				cchValueLeng = MAX_KEY_LENGTH;
				achValue.clear();
				achValue.resize(MAX_KEY_LENGTH);
			}
		}
	}

	return result;
}

bool hmc_registr_util::createRegistrValueKey(HKEY hKey, const std::wstring FolderPath, std::wstring ValueKey, DWORD ValueType)
{

	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		bool temp = ::RegSetValueExW(open_hkey, ValueKey.c_str(), 0, ValueType, nullptr, 0) == ERROR_SUCCESS;
		result = temp;
	}

	return result;
}

bool hmc_registr_util::createRegistrValueKey(HKEY hKey, const std::string FolderPath, std::string ValueKey, DWORD ValueType)
{

	bool result = false;
	HKEY open_hkey = nullptr;

	HMC_FreeRegKeyAuto(open_hkey);

	if (ERROR_SUCCESS == ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey))
	{
		bool temp = ::RegSetValueExA(open_hkey, ValueKey.c_str(), 0, ValueType, nullptr, 0) == ERROR_SUCCESS;
		result = temp;
	}
	return result;
}

std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getFolderKeys(){
	return hmc_registr_util::chQueryFolderInfoA::getFolderKeys(this->folderKeyList);
}
std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getValueKeys()
{
	return hmc_registr_util::chQueryFolderInfoA::getValueKeys(this->valueKeyList);
}
std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getFolderKeys(std::vector<hmc_registr_util::chQueryFolderKeyItemA> folderKeyList)
{
	std::vector<std::string> keyList;
	for (size_t i = 0; i < folderKeyList.size(); i++)
	{
		auto vkey = folderKeyList.at(i);
		keyList.push_back(vkey.name);
	}
	return keyList;
}
std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getValueKeys(std::vector<hmc_registr_util::chQueryValueKeyItemA> valueKeyList)
{
	std::vector<std::string> keyList;
	for (size_t i = 0; i < valueKeyList.size(); i++)
	{
		auto vkey = valueKeyList.at(i);
		keyList.push_back(vkey.name);
	}
	return keyList;
}
std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getFolderKeys(hmc_registr_util::chQueryFolderInfoA QueryFolderInfo)
{
	return hmc_registr_util::chQueryFolderInfoA::getFolderKeys(QueryFolderInfo.folderKeyList);
}
std::vector<std::string> hmc_registr_util::chQueryFolderInfoA::getValueKeys(hmc_registr_util::chQueryFolderInfoA QueryFolderInfo)
{
	return hmc_registr_util::chQueryFolderInfoA::getValueKeys(QueryFolderInfo.valueKeyList);
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getFolderKeys()
{
	return hmc_registr_util::chQueryFolderInfoW::getFolderKeys(this->folderKeyList);
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getValueKeys()
{
	return hmc_registr_util::chQueryFolderInfoW::getValueKeys(this->valueKeyList);
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getFolderKeys(std::vector<hmc_registr_util::chQueryFolderKeyItemW> folderKeyList)
{
	std::vector<std::wstring> keyList;
	for (size_t i = 0; i < folderKeyList.size(); i++)
	{
		auto vkey = folderKeyList.at(i);
		keyList.push_back(vkey.name);
	}
	return keyList;
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getValueKeys(std::vector<hmc_registr_util::chQueryValueKeyItemW> valueKeyList)
{
	std::vector<std::wstring> keyList;
	for (size_t i = 0; i < valueKeyList.size(); i++)
	{
		auto vkey = valueKeyList.at(i);
		keyList.push_back(vkey.name);
	}
	return keyList;
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getFolderKeys(hmc_registr_util::chQueryFolderInfoW QueryFolderInfo)
{
	return hmc_registr_util::chQueryFolderInfoW::getFolderKeys(QueryFolderInfo.folderKeyList);
}
std::vector<std::wstring> hmc_registr_util::chQueryFolderInfoW::getValueKeys(hmc_registr_util::chQueryFolderInfoW QueryFolderInfo)
{
	return hmc_registr_util::chQueryFolderInfoW::getValueKeys(QueryFolderInfo.valueKeyList);
}

hmc_registr_util::chValueData hmc_registr_util::getRegistrBuffer(HKEY hKey, std::string FolderPath, std::string KeyName)
{
	chValueData result = {0, 0, 0, {}};
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	LSTATUS status = ERROR_SUCCESS;

	if ((status = ::RegOpenKeyExA(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey)) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD valueType;
	DWORD pDataSize = 0;

	// 获取值的大小，值类型
	if ((status = ::RegQueryValueExA(open_hkey, KeyName.c_str(), nullptr, &valueType, nullptr, &pDataSize)) != ERROR_SUCCESS)
	{
		return result;
	}

	result.size = pDataSize;
	result.type = valueType;
	result.exists = true;

	// push data 超过4G可能是溢出了？
	if (pDataSize <= 1 || pDataSize > 0xffffffff)
	{
		return result;
	}

	result.value.resize(static_cast<long long>(pDataSize) + 1024);

	if ((status = ::RegQueryValueExA(open_hkey, KeyName.c_str(), 0, &valueType, reinterpret_cast<BYTE *>(result.value.data()), &pDataSize)) == ERROR_SUCCESS)
	{
		result.value.resize(pDataSize);
	}

	return result;
}

hmc_registr_util::chValueData hmc_registr_util::getRegistrBuffer(HKEY hKey, std::wstring FolderPath, std::wstring KeyName)
{
	chValueData result = {0, 0, 0, {}};
	HKEY open_hkey = nullptr;
	HMC_FreeRegKeyAuto(open_hkey);

	LSTATUS status = ERROR_SUCCESS;

	if ((status = ::RegOpenKeyExW(hKey, FolderPath.c_str(), 0, KEY_ALL_ACCESS, &open_hkey)) != ERROR_SUCCESS)
	{
		return result;
	}

	DWORD valueType;
	DWORD pDataSize = 0;

	// 获取值的大小，值类型
	if ((status = ::RegQueryValueExW(open_hkey, KeyName.c_str(), nullptr, &valueType, nullptr, &pDataSize)) != ERROR_SUCCESS)
	{
		return result;
	}

	result.size = pDataSize;
	result.type = valueType;
	result.exists = true;

	// push data 超过4G可能是溢出了？
	if (pDataSize <= 1 || pDataSize > 0xffffffff)
	{
		return result;
	}

	result.value.resize(static_cast<long long>(pDataSize) + 1024);

	if ((status = ::RegQueryValueExW(open_hkey, KeyName.c_str(), 0, &valueType, reinterpret_cast<BYTE *>(result.value.data()), &pDataSize)) == ERROR_SUCCESS)
	{
		result.value.resize(pDataSize);
	}

	return result;
}

