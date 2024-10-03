#include <iostream>
#include <hmc-module.h>

int main()
{
	std::wstring temp = L"E:\\德云社2024年演出合集\\03.11.mp4";

	auto data = hmc_string_util::split(temp,'\\');

	auto _t = hmc_string_util::join(data,L"\\");
	std::vector<std::string> QueryFolderList;
	std::vector<std::string> QueryKeyList;

	//HKEY open_hkey = nullptr;

	//HMC_FreeRegKeyAuto(open_hkey);
	//bool is_open = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" , 0, KEY_ALL_ACCESS, &open_hkey) == ERROR_SUCCESS;
	//if (is_open)
	//{
	//	auto _b =  hmc_registr_util::getRegistrKeys(open_hkey, QueryFolderList, QueryKeyList);

	//	return 0;
	//}


	//auto _b = hmc_registr_util::getRegistrKeys(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QueryFolderList, QueryKeyList);


	auto _queryRegistrFolderInfo = hmc_registr_util::queryRegistrFolderInfo(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",true,true);

	std::wstring json_data = _queryRegistrFolderInfo.to_json();

	//hmc_util::writeFile("2222.json", hmc_string_util::utf16_to_utf8(json_data));
	hmc_util::writeFile(L"3333.json", json_data);
	//hmc_util::writeFile("3333.json", json_data);

	//std::wcout << "_queryRegistrFolderInfo-> "<< json_data << std::endl;
	for (size_t i = 0; i < _queryRegistrFolderInfo.folderKeyList.size(); i++)
	{
		auto it = _queryRegistrFolderInfo.folderKeyList.at(i);

		std::wcout <<" [ " << i <<" ] " << it.name <<" time -> " << it.time << std::endl;

	}

	for (size_t i = 0; i < _queryRegistrFolderInfo.valueKeyList.size(); i++)
	{
		auto it = _queryRegistrFolderInfo.valueKeyList.at(i);

		std::wcout << " [ " << i << " ] " << it.name << " size -> " << it.size <<"   [ " << it.type<< " ]  @ "<<"["<< hmc_registr_util::type_nameW(it.type) <<"]" << std::endl;

	}
	
	return 0;
}
