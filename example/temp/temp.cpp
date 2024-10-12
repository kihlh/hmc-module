#include <iostream>
#include <any>
#include <functional>
#include <hmc-module.h>


int main()
{
	using String = std::wstring;
	
	String ddd = String(LR"(F:\\Chatbox.lnk)");
	while (true)
	{
		std::wstring lnkPath = ddd;
		std::wstring path; 
		int64_t showCmd = 0;
		std::wstring args ; 
		std::wstring desc ; 
		std::wstring icon ; 
		int64_t iconIndex = 0; 
		std::wstring cwd ;
		int64_t hotkey = 0;

		bool isv = hmc_shell_util::getShortcutLink(lnkPath, &path, &showCmd, &args, &desc, &icon, &iconIndex, &cwd, &hotkey);

		if (isv) {

		}
		//std::cout << hmc_string_util::utf16_to_ansi(lnkPath) << " -> " << hmc_string_util::utf16_to_ansi(path) << std::endl;

	}
	return 0;
}
